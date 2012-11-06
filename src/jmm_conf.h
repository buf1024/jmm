/*
 * jmm_conf.h
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_JMM_CONF_H__
#define __48SLOTS_JMM_CONF_H__

#include "jmm_cmmhdr.h"

typedef struct jmm_conf
{
    int log_term_lvl;
    int log_file_lvl;
    char log_file_path[JMM_MAX_PATH];

    int net_port;
    int net_backlog;
    int proc_num;
    int proc_svr_num;
    char shm_path[JMM_MAX_PATH];

    char conf_path[JMM_MAX_PATH];
}jmm_conf;

// 从文件初始化
int jmm_init_conf(jmm_conf* conf);
// 初始化默认配置
int jmm_init_def_conf(jmm_conf* conf);

#ifdef DEBUG
void jmm_trace_conf(jmm_conf* conf, int ls);
#endif

#endif /* __48SLOTS_JMM_CONF_H__ */
