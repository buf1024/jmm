/*
 * jmm_conf.c
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#include "jmm_conf.h"
#include "ciniconfig.h"

// read ini macro
#define JMM_GET_SESSION_MUST(ini, sec, name)                   \
do{                                                            \
    sec = (ini)->get_section((ini), name);                     \
    if(!sec){                                                  \
        fprintf(stderr, "fail to get section: %s\n", name);    \
        uninit_ini((ini));                                     \
        return JMM_FAIL;                                       \
    }                                                          \
}while(0)                                                      \

#define JMM_GET_SESSION_OPT(ini, sec, name)                    \
do{                                                            \
    sec = (ini)->get_section((ini), name);                     \
    if(!sec){                                                  \
        fprintf(stderr, "fail to get section: %s\n", name);    \
    }                                                          \
}while(0)                                                      \

#define JMM_GET_INT_MUST(ini, sec, key, value)                 \
do{                                                            \
    if(sec->get_int(sec, key, value) != 0){                    \
        fprintf(stderr,                                        \
                "fail to get int value: section:%s, key=%s\n", \
                sec->name, key);                               \
        uninit_ini((ini));                                     \
        return JMM_FAIL;                                       \
    }                                                          \
}while(0)                                                      \

#define JMM_GET_INT_OPT(ini, sec, key, value)                  \
do{                                                            \
    if(sec->get_int(sec, key, value) != 0){                    \
        fprintf(stderr,                                        \
                "fail to get int value: section:%s, key=%s\n", \
                sec->name, key);                               \
    }                                                          \
}while(0)                                                      \

#define JMM_GET_STRING_MUST(ini, sec, key, value, len)         \
do{                                                            \
    if(sec->get_string(sec, key, value, len) != 0){            \
        fprintf(stderr,                                        \
              "fail to get string value: section:%s, key=%s\n",\
              sec->name, key);                                 \
        uninit_ini((ini));                                     \
        return JMM_FAIL;                                       \
    }                                                          \
}while(0)                                                      \

#define JMM_GET_STRING_OPT(ini, sec, key, value, len)          \
do{                                                            \
    if(sec->get_string(sec, key, value, len) != 0){            \
        fprintf(stderr,                                        \
            "fail to get string value: section:%s, key=%s\n",  \
            sec->name, key);                                   \
    }                                                          \
}while(0)                                                      \

/*
[COMMON]
LOG_TERM_LEVEL=debug
LOG_FILE_LEVEL=debug
LOG_FILE_PATH=log/jmm
NET_PORT=10433
NET_BACKLOG=128
PROC_NUM=8
PROC_SVR_NUM=1
SHM_PATH=conf/jmm_shm
*/

// configure item
#define JMM_CONF_SESSION                                       "COMMON"
#define JMM_CONF_LOG_TERM_LVL                                  "LOG_TERM_LEVEL"
#define JMM_CONF_LOG_FILE_LVL                                  "LOG_FILE_LEVEL"
#define JMM_CONF_LOG_FILE_PATH                                 "LOG_FILE_PATH"
#define JMM_CONF_NET_PORT                                      "NET_PORT"
#define JMM_CONF_NET_BACKLOG                                   "NET_BACKLOG"
#define JMM_CONF_PROC_NUM                                      "PROC_NUM"
#define JMM_CONF_PROC_SVR_NUM                                  "PROC_SVR_NUM"
#define JMM_CONF_SHM_PATH                                      "SHM_PATH"

// default value
#define JMM_CONF_DEF_LOG_TERM_LVL                              0
#define JMM_CONF_DEF_LOG_FILE_LVL                              0
#define JMM_CONF_DEF_LOG_FILE_PATH                             "log/jmm"
#define JMM_CONF_DEF_NET_PORT                                  10433
#define JMM_CONF_DEF_NET_BACKLOG                               128
#define JMM_CONF_DEF_PROC_NUM                                  2
#define JMM_CONF_DEF_PROC_SVR_NUM                              2
#define JMM_CONF_DEF_SHM_PATH                                  "conf/jmm_shm"
#define JMM_CONF_DEF_CONF_PATH                                 "conf/jmm.conf"

int jmm_init_conf(jmm_conf* conf)
{
    if(!conf){
        return JMM_FAIL;
    }
    struct ini_config ini;
    struct section* sec;

    const char* file_new = 0;
    char value[JMM_MAX_NUM_STRING] = "";
    init_ini(&ini);
    if (ini.load(&ini, conf->conf_path) < 0) {
        if (ini.load(&ini, JMM_CONF_DEF_CONF_PATH) < 0) {
            return JMM_FAIL;
        }
        file_new = JMM_CONF_DEF_CONF_PATH;
    }else{
        file_new = conf->conf_path;
    }

    JMM_GET_SESSION_MUST(&ini, sec, JMM_CONF_SESSION);

    //log
    memset(value, 0, sizeof(value));
    JMM_GET_STRING_MUST(&ini, sec, JMM_CONF_LOG_TERM_LVL, value, JMM_MAX_NUM_STRING-1);
    conf->log_term_lvl = clog_get_level(value);
    memset(value, 0, sizeof(value));
    JMM_GET_STRING_MUST(&ini, sec, JMM_CONF_LOG_FILE_LVL, value, JMM_MAX_NUM_STRING-1);
    conf->log_file_lvl = clog_get_level(value);
    JMM_GET_STRING_MUST(&ini, sec, JMM_CONF_LOG_FILE_PATH, conf->log_file_path, JMM_MAX_PATH-1);
    //net
    JMM_GET_INT_MUST(&ini, sec, JMM_CONF_NET_PORT, &(conf->net_port));
    JMM_GET_INT_MUST(&ini, sec, JMM_CONF_NET_BACKLOG, &(conf->net_backlog));
    //proc
    JMM_GET_INT_MUST(&ini, sec, JMM_CONF_PROC_NUM, &(conf->proc_num));
    JMM_GET_INT_MUST(&ini, sec, JMM_CONF_PROC_SVR_NUM, &(conf->proc_svr_num));
    //shm
    JMM_GET_STRING_MUST(&ini, sec, JMM_CONF_SHM_PATH, conf->shm_path, JMM_MAX_PATH-1);

    if(file_new != conf->conf_path){
        strncpy(conf->conf_path, file_new, JMM_MAX_PATH-1);
    }


    uninit_ini(&ini);

    return JMM_SUCCESS;
}
int jmm_init_def_conf(jmm_conf* conf)
{
    if(!conf){
        return JMM_FAIL;
    }

    conf->log_file_lvl = JMM_CONF_DEF_LOG_FILE_LVL;
    conf->log_file_lvl = JMM_CONF_DEF_LOG_FILE_LVL;
    strcpy(conf->log_file_path, JMM_CONF_DEF_LOG_FILE_PATH);

    conf->net_port = JMM_CONF_DEF_NET_PORT;
    conf->net_backlog = JMM_CONF_DEF_NET_BACKLOG;
    conf->proc_num = JMM_CONF_DEF_PROC_NUM;
    conf->proc_svr_num = JMM_CONF_DEF_PROC_SVR_NUM;
    strcpy(conf->shm_path, JMM_CONF_DEF_SHM_PATH);


    return JMM_SUCCESS;
}











#ifdef DEBUG
void jmm_trace_conf(jmm_conf* conf, int ls)
{
    if(conf){
        jmm_print_space(ls);
        printf("=====jmm_conf====\n");
        jmm_print_space(ls);
        printf("log_term_lvl : %d\n", conf->log_term_lvl);
        jmm_print_space(ls);
        printf("log_file_lvl : %d\n", conf->log_file_lvl);
        jmm_print_space(ls);
        printf("log_file_path : %s\n", conf->log_file_path);
        jmm_print_space(ls);
        printf("net_port : %d\n", conf->net_port);
        jmm_print_space(ls);
        printf("net_backlog : %d\n", conf->net_backlog);
        printf("proc_num : %d\n", conf->proc_num);
        jmm_print_space(ls);
        printf("proc_svr_num : %d\n", conf->proc_svr_num);
        jmm_print_space(ls);
        printf("shm_path : %s\n", conf->shm_path);
        jmm_print_space(ls);
        printf("conf_path : %s\n", conf->conf_path);

    }
}
#endif


