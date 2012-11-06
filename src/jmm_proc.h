/*
 * jmm_proc.h
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_JMM_PROC_H__
#define __48SLOTS_JMM_PROC_H__

#include "jmm_conf.h"

// 初始化和销毁进程池
int jmm_init_proc(jmm_conf* conf);
int jmm_uninit_proc();

// 查找可供服务的子进程
int jmm_find_free_wf(int* wf_id, int* sock_id);
// 将socket传递到相关的子进程中
int jmm_assign_wf(int wf_id, int sock_id, int sock_fd, const char* ip, int port);

// 子进程清空非必要的资源
int jmm_proc_clear_env_wf();
// 子进程服务进程
int jmm_proc_wf(int wf_id);

#endif /* __48SLOTS_JMM_PROC_H__ */
