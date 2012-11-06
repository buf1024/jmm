/*
 * jmm_cmmhdr.h
 *
 *  Created on: 2012-5-20
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_JMM_CMMHDR_H__
#define __48SLOTS_JMM_CMMHDR_H__

#include "jmm_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define JMM_SUCCESS                 0
#define JMM_FAIL                    -1

#define JMM_TRUE                    1
#define JMM_FALSE                   0

#define JMM_MAX_PATH                256
#define JMM_MAX_NAME                64
#define JMM_MAX_NUM_STRING          32
#define JMM_MAX_ADDR                64
#define JMM_MAX_SOCK_RECV_SIZE      2048
#define JMM_MAX_SOCK_SEND_SIZE      2048
#define JMM_SHARE_MEM_ID            0x1f



#define JMM_DEBUG(...)              CLOG_DEBUG(__VA_ARGS__)
#define JMM_INFO(...)               CLOG_INFO(__VA_ARGS__)
#define JMM_WARN(...)               CLOG_WARN(__VA_ARGS__)
#define JMM_ERROR(...)              CLOG_ERROR(__VA_ARGS__)
#define JMM_FATAL(...)              CLOG_FATAL(__VA_ARGS__)



#define JMM_MAX(a, b)               ((a)>(b)?(a):(b))
#define JMM_MIN(a, b)               ((a)>(b)?(b):(a))

#endif /* __48SLOTS_JMM_CMMHDR_H__ */
