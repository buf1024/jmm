/*
 * jmm_event.c
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#include "jmm_cmmhdr.h"
#include "jmm_event.h"
#include "jmm_conf.h"
#include "jmm_proc.h"
#include "jmm_shm.h"
#include "jmm_util.h"
#include "jmm.h"
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>

jmm_event                    event = {0};
jmm_event_wf              event_wf = {0};
extern jmm_conf               conf;
extern jmm_shm*                shm;
extern jmm_shm_wf*          shm_wf;
extern jmm_hook            hook_fn;

// 单个进程只要一个接收缓冲就可以了
static char          sock_recv_buf[JMM_MAX_SOCK_RECV_SIZE] = "";

// 控制进程事件回调
static void jmm_ctrl_c_cb(evutil_socket_t fd, short what, void* ctx);
static void jmm_usr1_cb(evutil_socket_t fd, short what, void* ctx);
static void jmm_child_cb(evutil_socket_t fd, short what, void* ctx);
static void jmm_listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
    struct sockaddr* sa, int socklen, void* user_data);

// 子进程事件回调
static void jmm_ctrl_c_wf_cb(evutil_socket_t fd, short what, void* ctx);
static void jmm_usr1_wf_cb(evutil_socket_t fd, short what, void* ctx);

// 子进程和控制进程通信事件回调
static void jmm_cmm_read_wf_cb(evutil_socket_t fd, short what, void* ctx);

// 子进程 socket 事件回调
static void jmm_sock_read_wf_cb(struct bufferevent*, void *);
static void jmm_sock_write_wf_cb(struct bufferevent*, void*);
static void jmm_sock_event_wf_cb(struct bufferevent*, short, void*);

// 控制进程事件
int jmm_init_event(struct event_base* base)
{
    if (base) {
        event.base = base;
        // SIGTERM
        event.term = evsignal_new(base, SIGTERM, jmm_ctrl_c_cb, (void *)base);
        if (!event.term || event_add(event.term, NULL) < 0) {
            return JMM_FAIL;
        }
        // CTRL-C
        event.ctrl_c = evsignal_new(base, SIGINT, jmm_ctrl_c_cb, (void *)base);
        if (!event.ctrl_c || event_add(event.ctrl_c, NULL) < 0) {
            return JMM_FAIL;
        }
        // USR1
        event.usr1 = evsignal_new(base, SIGUSR1, jmm_usr1_cb, (void *)base);
        if (!event.usr1 || event_add(event.usr1, NULL) < 0) {
            return JMM_FAIL;
        }
        // CHLD
        event.child = evsignal_new(base, SIGCHLD, jmm_child_cb, (void *)base);
        if (!event.child || event_add(event.child, NULL) < 0) {
            return JMM_FAIL;
        }

        // 监听接口
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(conf.net_port);

        event.listener = evconnlistener_new_bind(base, jmm_listener_cb, (void *)base,
            LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_CLOSE_ON_EXEC,
            conf.net_backlog, (struct sockaddr*)&sin, sizeof(sin));

        if (!event.listener) {
            return JMM_FAIL;
        }
        return JMM_SUCCESS;
    }
    return JMM_FAIL;
}
int jmm_uninit_event()
{
    if(event.listener){
        evconnlistener_free(event.listener);
        event.listener = NULL;
    }
    if(event.child){
        event_free(event.child);
        event.child = NULL;
    }
    if(event.ctrl_c){
        event_free(event.ctrl_c);
        event.ctrl_c = NULL;
    }
    if(event.usr1){
        event_free(event.usr1);
        event.usr1 = NULL;
    }
    if (event.term) {
        event_free(event.term);
        event.term = NULL;
    }

    return JMM_SUCCESS;
}

// 子进程事件
int jmm_init_event_wf(struct event_base* base)
{
    if (base) {
        event_wf.base = base;
        // SIGTERM
        event_wf.term =
                evsignal_new(base, SIGTERM, jmm_ctrl_c_wf_cb, (void *)base);
        if (!event_wf.term || event_add(event_wf.term, NULL) < 0) {
            return JMM_FAIL;
        }
        // CTRL-C
        event_wf.ctrl_c = evsignal_new(base, SIGINT, jmm_ctrl_c_wf_cb, (void *)base);
        if (!event_wf.ctrl_c || event_add(event_wf.ctrl_c, NULL) < 0) {
            return JMM_FAIL;
        }
        // USR1
        event_wf.usr1 = evsignal_new(base, SIGUSR1, jmm_usr1_wf_cb, (void *)base);
        if (!event_wf.usr1 || event_add(event_wf.usr1, NULL) < 0) {
            return JMM_FAIL;
        }

        // CMM
        event_wf.cmm = event_new(base, shm_wf->girl_fd, EV_READ | EV_PERSIST,
                jmm_cmm_read_wf_cb, NULL);
        if (!event_wf.cmm || event_add(event_wf.cmm, NULL) < 0) {
            return JMM_FAIL;
        }

        //sock
        event_wf.sock_num = conf.proc_svr_num;
        event_wf.sock = (struct bufferevent**)malloc(sizeof(struct bufferevent*)*event_wf.sock_num);
        memset(event_wf.sock, 0, sizeof(struct bufferevent*)*event_wf.sock_num);

        return JMM_SUCCESS;
    }
    return JMM_FAIL;
}
int jmm_uninit_event_wf()
{
    if (event_wf.sock) {
        int i = 0;
        for (i = 0; i < event_wf.sock_num; i++) {
            if (event_wf.sock[i] != NULL) {
                bufferevent_free(event_wf.sock[i]);
                event_wf.sock[i] = NULL;
            }
        }
        free(event_wf.sock);
        event_wf.sock = NULL;
        event_wf.sock_num = 0;
    }

    if(event_wf.cmm){
        event_free(event_wf.cmm);
        event_wf.cmm = NULL;
    }
    if(event_wf.ctrl_c){
        event_free(event_wf.ctrl_c);
        event_wf.ctrl_c = NULL;
    }
    if(event_wf.usr1){
        event_free(event_wf.usr1);
        event_wf.usr1 = NULL;
    }
    if(event_wf.term){
        event_free(event_wf.term);
        event_wf.term = NULL;
    }

    return JMM_SUCCESS;
}

// socket事件
int jmm_init_event_sock(int idx, int sfd)
{
    if(event_wf.sock[idx] != NULL){
        return JMM_FAIL;
    }
    event_wf.sock[idx] = bufferevent_socket_new(event_wf.base, sfd,
            BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(event_wf.sock[idx], jmm_sock_read_wf_cb, jmm_sock_write_wf_cb,
            jmm_sock_event_wf_cb, (void*)idx);
    bufferevent_enable(event_wf.sock[idx], EV_WRITE);
    bufferevent_enable(event_wf.sock[idx], EV_READ);

    return JMM_SUCCESS;
}

// 控制进程CTRL-C
static void jmm_ctrl_c_cb(evutil_socket_t fd, short what, void* ctx)
{
    JMM_INFO("catch terminate signal.\n");

    struct event_base *base = ctx;
    if(shm){
        int i = 0;
        for(i=0; i<shm->proc_num; i++){
            // 结束子进程
            jmm_shm_wf* wf = jmm_shm_get_wf(shm, i, conf.proc_svr_num);
            if (wf->pid != 0) {
                kill(wf->pid, SIGINT);
                waitpid(wf->pid, NULL, 0);
            }
            wf->pid = 0;
        }
    }

    event_base_loopbreak(base);
}
// 控制进程USR1
static void jmm_usr1_cb(evutil_socket_t fd, short what, void* ctx)
{
    JMM_INFO("catch usr1 signal.\n");
}
// 控制进程CHD
static void jmm_child_cb(evutil_socket_t fd, short what, void* ctx)
{
    // 除非主进程要子进程断，否则子进程不应该自己停止
    // 目前只将池减少一下
    JMM_INFO("catch child terminate signal.\n");
    pid_t pid;
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0){
        JMM_INFO("child process exit, pid=%d\n", pid);
        // pid == 0, 表示不可用
        int i = 0;
        for (i = 0; i < shm->proc_num; i++) {
            jmm_shm_wf* wf = jmm_shm_get_wf(shm, i, conf.proc_svr_num);
            if(wf->pid == pid){
                wf->pid = 0;
                break;
            }
        }
    }
}
// 控制进程监听网络
static void jmm_listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{
    char ip_addr[JMM_MAX_ADDR] = "";
    int port = 0;
    if(socklen == sizeof(struct sockaddr_in)){
        struct sockaddr_in* sa_in = (struct sockaddr_in*)sa;
        inet_ntop(AF_INET, &(sa_in->sin_addr), ip_addr, sizeof(ip_addr));
        port = sa_in->sin_port;
        JMM_INFO("get connection: (%s, %d)\n", ip_addr, sa_in->sin_port);
    }else{
        struct sockaddr_in6* sa_in6 = (struct sockaddr_in6*)sa;
        inet_ntop(AF_INET6, &(sa_in6->sin6_addr), ip_addr, sizeof(ip_addr));
        port = sa_in6->sin6_port;
        JMM_INFO("get connection: (%s, %d)\n", ip_addr, sa_in6->sin6_port);
    }

    // 寻找可用的子进程
    int wf_id   = 0;
    int sock_id = 0;;
    if(jmm_find_free_wf(&wf_id, &sock_id) == JMM_FAIL){
        JMM_INFO("no available resource!\n");
    } else {
        if (jmm_assign_wf(wf_id, sock_id, fd, ip_addr, port) == JMM_FAIL) {
            JMM_INFO("fail to assign program!\n");
        }
    }
    //如果不成功指派则关闭，但如果如果成功指派也要关闭，因为已经飞到子进程中了
    close(fd);
}

static void jmm_ctrl_c_wf_cb(evutil_socket_t fd, short what, void* ctx)
{
    struct event_base *base = ctx;


    event_base_loopbreak(base);
}
static void jmm_usr1_wf_cb(evutil_socket_t fd, short what, void* ctx)
{
}

static void jmm_cmm_read_wf_cb(evutil_socket_t fd, short what, void* ctx)
{
    if(what & EV_READ){
        int sock_id = 0;
        read(fd, &sock_id, sizeof(int));
        JMM_DEBUG("jmm_cmm_read_wf_cb, sock_id: %d\n", sock_id);
        jmm_shm_sock* shm_sock = jmm_shm_get_sock(shm_wf, sock_id);
        if (shm_sock->sock_fd == 0) {
            int newsf=-1;
            jmm_recv_fd(shm_wf->mother_fd, &newsf);
            jmm_set_fd_opt(newsf, O_NONBLOCK);
            jmm_init_event_sock(sock_id, newsf);

            shm_sock->sock_fd = newsf;

            JMM_DEBUG("jmm_cmm_read_wf_cb, sock_fd: %d\n", newsf);
            // 子进程解父进程锁
            pthread_mutex_unlock(&(shm_wf->mutex));
        }else{
            JMM_DEBUG("should never go here: sock_fd=%d\n", shm_sock->sock_fd);
        }
    }
}

// socket
static void jmm_sock_read_wf_cb(struct bufferevent* bev, void* ctx)
{
    struct evbuffer *input = bufferevent_get_input(bev);
    int len = 0;
    int actual = 0;
    char* data = sock_recv_buf;
    while ((len = evbuffer_get_length(input)) != 0) {
        while (len != 0) {
            if(len > JMM_MAX_SOCK_RECV_SIZE){
                evbuffer_remove(input, data, JMM_MAX_SOCK_RECV_SIZE);
                actual = JMM_MAX_SOCK_RECV_SIZE;
                len -= JMM_MAX_SOCK_RECV_SIZE;
            }else{
                evbuffer_remove(input, data, len);
                actual = len;
                len = 0;
            }
            if(hook_fn.prog_service){
                jmm_prog_in in = {0};
                jmm_prog_out out = {0};

                in.len = actual;
                in.bytes = data;

                if(hook_fn.prog_service(&in, &out) != JMM_SUCCESS){
                    // FATAL ERROR
                    JMM_FATAL("server encounter fatal error!\n");
                    event_base_loopbreak(event_wf.base);
                }else{
                    if(out.status == JMM_SUCCESS && out.len > 0){
                        bufferevent_write(bev, out.bytes, out.len);
                    }else{
                        JMM_ERROR("business error!\n");
                    }
                }
            }
        }

    }

}
static void jmm_sock_write_wf_cb(struct bufferevent* bev, void* ctx)
{
    struct evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
    }
}
static void jmm_sock_event_wf_cb(struct bufferevent* bev, short what, void* ctx)
{
    if (what & BEV_EVENT_EOF) {
        JMM_INFO("connection closed.\n");
    } else if (what & BEV_EVENT_ERROR) {
        JMM_ERROR("got an error on the connection: %s\n", strerror(errno));
    }
    int idx = (int)(ctx);
    pthread_mutex_lock(&(shm_wf->mutex));
    bufferevent_free(event_wf.sock[idx]);
    event_wf.sock[idx] = NULL;
    jmm_shm_sock* sock = jmm_shm_get_sock(shm_wf, idx);
    sock->sock_fd = 0;
    pthread_mutex_unlock(&(shm_wf->mutex));
}
