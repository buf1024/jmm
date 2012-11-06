/*
 * jmm.c
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#include "jmm_cmmhdr.h"
#include "jmm_conf.h"
#include "jmm_util.h"
#include "jmm_event.h"
#include "jmm_shm.h"
#include "jmm_proc.h"
#include "jmm.h"
#include <getopt.h>
#include <event2/event.h>

#define JMM_PROG_DEF_NAME           "jmm"
#define JMM_PROG_DEF_VERSION        "0.01"
#define JMM_PROG_DEF_DESC           "scratch server"

jmm_hook                        hook_fn = {0};
jmm_conf                           conf = {0};

extern prog_init_hook prog_init_hook_fn;

static void jmm_init_env();
static void jmm_uninit_env();

static void jmm_version();
static void jmm_usage();

int main(int argc, char **argv)
{
    struct event_base* base = NULL;
    const char* optstr = "c:ehv";
    struct option optlong[] = {
            {"configure", 1, NULL, 'c'},
            {"exclude", 0, NULL, 'e'},
            {"version", 0, NULL, 'v'},
            {"help", 0, NULL, 'h'},
            {NULL, 0, NULL, 0}
    };
    int daemon = JMM_TRUE;
    int opt;

    jmm_init_env();

    // no root
    if(jmm_is_runas_root() == JMM_TRUE){
        fprintf(stderr, "%s is not allow to run as root!\n", hook_fn.prog_name());
        exit(JMM_FAIL);
    }

    while ((opt = getopt_long(argc, argv, optstr, optlong, NULL)) != -1) {
        switch (opt) {
        case 'c':
            strncpy(conf.conf_path, optarg, JMM_MAX_PATH - 1);
            break;
        case 'e':
            daemon = JMM_FALSE;
            break;
        case 'v':
            jmm_version();
            exit(JMM_SUCCESS);
            break;
        case 'h':
            jmm_usage();
            exit(JMM_SUCCESS);
            break;
        case ':':
        case '?':
            jmm_usage();
            exit(JMM_FAIL);
            break;
        default:
            break;
        }
    }

    if(daemon){
        jmm_daemonlize();
    }

    if(strlen(conf.conf_path) > 0){
        if(jmm_init_conf(&conf) == JMM_FAIL){
            fprintf(stderr, "fail to load configure from file: %s\n", conf.conf_path);
            exit(JMM_FAIL);
        }
    }else{
        jmm_init_def_conf(&conf);
        fprintf(stdout, "configure file not specific, use the default.\n");
    }

    base = event_base_new();
    if(!base){
        fprintf(stderr, "event_base_new failed\n");
        exit(JMM_FAIL);
    }
    //log
    if(jmm_init_log() != JMM_SUCCESS){
        fprintf(stderr, "jmm_init_log failed\n");
        event_base_free(base);
        exit(JMM_FAIL);
    }
    JMM_INFO("logger is ready\n");
    //shm
    if(jmm_init_shm(&conf) != JMM_SUCCESS){
        JMM_FATAL("jmm_init_shm failed\n");
        jmm_uninit_log();
        event_base_free(base);
        exit(JMM_FAIL);
    }
    JMM_INFO("share memory is ready\n");
    //pool
    if(jmm_init_proc(&conf) != JMM_SUCCESS){
        JMM_FATAL("jmm_init_proc failed\n");
        jmm_uninit_shm();
        jmm_uninit_log();
        event_base_free(base);
        exit(JMM_FAIL);
    }
    JMM_INFO("process pool is ready\n");
    //event
    if(jmm_init_event(base) != JMM_SUCCESS){
        JMM_FATAL("jmm_init_event failed\n");
        jmm_uninit_proc();
        jmm_uninit_shm();
        jmm_uninit_log();
        event_base_free(base);
        exit(JMM_FAIL);
    }
    JMM_INFO("event is ready\n");
    JMM_INFO("control process entering event loop...\n");
    event_base_dispatch(base);

    JMM_INFO("free resource and exit\n");
    jmm_uninit_event();
    jmm_uninit_proc();
    jmm_uninit_shm();
    jmm_uninit_log();

    event_base_free(base);

    jmm_uninit_env();

    return JMM_SUCCESS;

}


static void jmm_init_env()
{
    if(prog_init_hook_fn){
        prog_init_hook_fn(&hook_fn);
    }
}
static void jmm_uninit_env()
{

}

static void jmm_version()
{
    printf("%s version : %s\n",
            hook_fn.prog_name?hook_fn.prog_name():JMM_PROG_DEF_NAME,
            hook_fn.prog_version?hook_fn.prog_version():JMM_PROG_DEF_VERSION);
}
static void jmm_usage()
{
    printf("%s ---- %s\n\n",
            hook_fn.prog_name?hook_fn.prog_name():JMM_PROG_DEF_NAME,
            hook_fn.prog_desc?hook_fn.prog_desc():JMM_PROG_DEF_DESC);
    printf("  -c, --configure=configuration file  Specific the configuration file\n");
    printf("                                      If not specific, use the default setting\n");
    printf("  -e, --exclude                       Don't start as daemon process\n");
    printf("  -v, --version                       Print the program version message\n");
    printf("  -h, --help                          Print this help message\n");
}

