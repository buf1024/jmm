/*
 * jmm_shm.h
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_JMM_SHM_H__
#define __48SLOTS_JMM_SHM_H__

#include "jmm_cmmhdr.h"
#include "jmm_conf.h"
#include <sys/types.h>

// 网络连接信息
typedef struct jmm_shm_sock
{
/*    time_t cnn_time;
    char cnn_ip[JMM_MAX_ADDR];
    int cnn_port;*/
    int sock_fd;   // 当前连接文件描述符，0表示无连接
}jmm_shm_sock;

// 业务子进程信息
typedef struct jmm_shm_wf
{
    pthread_mutex_t mutex; // 用于进程间同步
    pid_t pid;     // 进程id
    int father_fd; // 用于父进程向子进程传递文件描述符，因为和libevent混合在一起不行
    int mother_fd; // 用于子进程向父进程传递文件描述符，因为和libevent混合在一起不行

    int boy_fd;    // 父进程控制命令文件描述符
    int girl_fd;   // 子进程控制命令文件描述符


    //加变量在这之前加
    int proc_svr_num; // 子进程可服务的连接数
    jmm_shm_sock* shm_sock;
}jmm_shm_wf;

typedef struct jmm_shm
{
    pid_t pid;
    //加变量在这之前加
    int proc_num;
    jmm_shm_wf* shm_wf;
}jmm_shm;

size_t jmm_shm_size(int proc_num, int proc_svr_num);
size_t jmm_shm_wf_size(int proc_svr_num);
size_t jmm_shm_sock_size();
jmm_shm_wf* jmm_shm_get_wf(jmm_shm* theshm, int wf_id, int svr_num);
jmm_shm_sock* jmm_shm_get_sock(jmm_shm_wf* theshm, int sock_id);

int jmm_init_shm(jmm_conf* conf);
int jmm_uninit_shm();

int jmm_init_shm_wf(int wf_id, int wf_svr_num);
int jmm_uninit_shm_wf();

#ifdef DEBUG
void jmm_trace_shm(jmm_shm* theshm, int ls);
void jmm_trace_shm_wf(jmm_shm_wf* theshm, int ls);
void jmm_trace_shm_sock(jmm_shm_sock* theshm, int ls);
#endif

#endif /* __48SLOTS_JMM_SHM_H__ */
