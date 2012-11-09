/*
 * myserver.c
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#include "jmm.h"
#include "jmm_cmmhdr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char sock_send_buf[JMM_MAX_SOCK_SEND_SIZE] = "";

static const char* my_name()
{
    return "jmm";
}
static const char* my_version()
{
    return "0.01";
}
static const char* my_desc()
{
    return "process pool server";
}

static int my_init(char* conf)
{
    printf("function init!\n");
    return 0;
}
static int my_service(jmm_prog_in* in, jmm_prog_out* out)
{
    char data[2048] = "";
    strncpy(data, in->bytes, in->len);
    fprintf(stdout, "receive:%s\n", data);

    out->status = 0;
    out->bytes = sock_send_buf;
    memcpy(out->bytes, in->bytes, in->len);
    out->len = in->len;
    return 0;
}
int my_uninit()
{
    printf("function uninit!\n");
    return 0;
}

static void myserver_init_hook(jmm_hook* hook)
{
    hook->prog_desc = my_desc;
    hook->prog_name = my_name;
    hook->prog_version = my_version;

    hook->prog_init = my_init;
    hook->prog_uninit = my_uninit;
    hook->prog_service = my_service;
}

REG_INIT_HOOK_FUN(myserver_init_hook)
