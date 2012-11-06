/*
 * jmm_proc.c
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#include "jmm_cmmhdr.h"
#include "jmm_proc.h"
#include "jmm_shm.h"
#include "jmm_event.h"
#include "jmm_util.h"
#include "jmm.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <event2/event.h>
#include <event2/bufferevent.h>

extern jmm_shm*                  shm;
extern jmm_conf                 conf;
extern jmm_hook              hook_fn;

int jmm_find_free_wf(int* wf_id, int* sock_id)
{
    if(shm == NULL || wf_id == NULL || sock_id == NULL){
        return JMM_FAIL;
    }

    int i = 0;
    int j = 0;
    for(i=0; i<shm->proc_num; i++){
        jmm_shm_wf* wf = jmm_shm_get_wf(shm, i, conf.proc_svr_num);
        if(wf->pid == 0){
            // 当前这个进程挂了，无视之
            continue;
        }

        if(pthread_mutex_trylock(&(wf->mutex)) == EBUSY){
            if (i == shm->proc_num - 1) {
                // 顽强的尝试
                JMM_INFO("resource is lock, trying later\n");
                i = 0;
            }
            continue;
        }
        // 获得锁
        for(j=0; j<wf->proc_svr_num; j++){
            jmm_shm_sock* sock = jmm_shm_get_sock(wf, j);
            if(sock->sock_fd == 0){
                // 让子程序解锁
                JMM_DEBUG("ctrl: i=%d, j=%d\n", i, j);
                *wf_id = i; *sock_id = j;
                return JMM_SUCCESS;
            }
        }
        pthread_mutex_unlock(&(wf->mutex));
    }
    return JMM_FAIL;
}
int jmm_assign_wf(int wf_id, int sock_id, int sock_fd, const char* ip, int port)
{
    if(shm == NULL || ip == NULL){
        return JMM_FAIL;
    }
    jmm_shm_wf* wf = jmm_shm_get_wf(shm, wf_id, conf.proc_svr_num);

    // TODO

    jmm_send_fd(wf->father_fd, sock_fd);

    write(wf->boy_fd, &sock_id, sizeof(int));


    return JMM_SUCCESS;
}


int jmm_init_proc(jmm_conf* conf)
{
    if(conf == NULL){
        return JMM_FAIL;
    }
    int ret = JMM_SUCCESS;
    pid_t pid;
    int i = 0;
    int sf[2] = { 0 };

    for (i = 0; i < conf->proc_num; i++) {
        jmm_shm_wf* wf = jmm_shm_get_wf(shm, i, conf->proc_svr_num);
        // 传递控制信息
        memset(sf, 0, sizeof(sf));
        if (pipe(sf) != 0) {
            ret = JMM_FAIL;
            break;
        }
        wf->boy_fd = sf[1];
        wf->girl_fd = sf[0];
        // 传递文件描述符
        memset(sf, 0, sizeof(sf));
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sf) != 0) {
            ret = JMM_FAIL;
            break;
        }
        wf->father_fd = sf[1];
        wf->mother_fd = sf[0];

        if((pid = fork()) == 0){
            //TODO
            wf->pid = getpid();
            exit(jmm_proc_wf(i));
        }else if(pid > 0){
            //TODO
            //JMM_INFO("fork program i=%d\n", i);
        }else{
            ret = JMM_FAIL;
            break;
        }
    }
    if(JMM_FAIL == ret){
        //TODO
        JMM_FATAL("fail to fork program.\n");
        JMM_FATAL("you should adjust system configuration. proc_num: %d, proc_svr_num: %d\n",
                conf->proc_num, conf->proc_svr_num);
    }
    return ret;
}

int jmm_uninit_proc()
{
    if(shm){
        int wf_id = 0;
        for(wf_id = 0; wf_id < shm->proc_num; wf_id++){
            jmm_shm_wf* wf = jmm_shm_get_wf(shm, wf_id, conf.proc_svr_num);
            if (wf->pid != 0) {
                kill(wf->pid, SIGINT);
                waitpid(wf->pid, NULL, 0);
            }
            close(wf->father_fd);
            close(wf->mother_fd);

            close(wf->boy_fd);
            close(wf->girl_fd);
            wf->pid = 0;
        }
    }
    return JMM_SUCCESS;
}

int jmm_proc_clear_env_wf()
{
    if(jmm_uninit_event() == JMM_FAIL){
        return JMM_FAIL;
    }
    return JMM_SUCCESS;
}

int jmm_proc_wf(int wf_id)
{
    if(jmm_proc_clear_env_wf() == JMM_FAIL){
        fprintf(stderr, "fail to cleanup process environment!\n");
        return JMM_FAIL;
    }
    struct event_base* base = NULL;
    base = event_base_new();
    if(!base){
        fprintf(stderr, "fail to call event_base_new\n");
        return JMM_FAIL;
    }
    // log
    if(jmm_init_log_wf() == JMM_FAIL){
        fprintf(stderr, "fail to init log!\n");
        event_base_free(base);
        return JMM_FAIL;
    }
    JMM_INFO("child(%d) logger is ready!\n", wf_id);
    // shm
    if(jmm_init_shm_wf(wf_id, conf.proc_svr_num) == JMM_FAIL){
        jmm_uninit_log_wf();
        event_base_free(base);
        return JMM_FAIL;
    }
    JMM_INFO("child(%d) share memory is ready!\n", wf_id);

    // event
    if(jmm_init_event_wf(base) == JMM_FAIL){
        jmm_uninit_shm_wf();
        jmm_uninit_log_wf();
        event_base_free(base);
        return JMM_FAIL;
    }
    if(hook_fn.prog_init){
        if(hook_fn.prog_init(conf.conf_path) == JMM_FAIL){
            jmm_uninit_event_wf();
            jmm_uninit_shm_wf();
                    jmm_uninit_log_wf();
                    event_base_free(base);
                    return JMM_FAIL;
        }
    }
    JMM_INFO("child(%d) event is ready!\n", wf_id);
    JMM_INFO("child(%d) entering event loop...\n", wf_id);
    event_base_dispatch(base);

    JMM_INFO("free resource, exit\n");
    if(hook_fn.prog_uninit){
        hook_fn.prog_uninit();
    }
    jmm_uninit_event_wf();
    jmm_uninit_shm_wf();
    jmm_uninit_log_wf();
    event_base_free(base);

    return JMM_SUCCESS;
}
