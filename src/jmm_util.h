/*
 * jmm_util.h
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_JMM_UTIL_H__
#define __48SLOTS_JMM_UTIL_H__

#include <sys/types.h>

#define offset(type, field)                         \
    ((size_t)&(((type*)(0))->field))

void jmm_daemonlize();
int jmm_is_runas_root();
int jmm_set_fd_opt(int fd, int opt);

ssize_t jmm_send_fd(int fd,/* void* ptr, size_t nbytes, */int sendfd);
ssize_t jmm_recv_fd(int fd,/* void* ptr, size_t nbytes, */int* recvfd);



#ifdef DEBUG
void jmm_print_space(int ls);
#endif

#endif /* __48SLOTS_JMM_UTIL_H__ */
