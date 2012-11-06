/*
 * jmm_util.c
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#include "jmm_cmmhdr.h"
#include "jmm_util.h"
#include <sys/socket.h>
#include <fcntl.h>

void jmm_daemonlize()
{
    int pid = 0;
    if((pid = fork()) > 0)
    {
        exit(0);
    }
    setsid();
    if((pid = fork()) > 0)
    {
        exit(0);
    }
}

int jmm_is_runas_root()
{
    if(getuid() == 0 || geteuid() == 0){
        return JMM_TRUE;
    }
    return JMM_FALSE;
}

int jmm_set_fd_opt(int fd, int opt)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) < 0
            || fcntl(fd, F_SETFL, flags | opt) < 0) {
        return JMM_FAIL;
    }
    return JMM_SUCCESS;
}

ssize_t jmm_send_fd(int fd,/* void* ptr, size_t nbytes, */int sendfd)
{
    struct msghdr msg;
    struct iovec iov[1];

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }control_un;
    struct cmsghdr *cmptr;

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    cmptr = CMSG_FIRSTHDR(&msg);
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    *((int*)CMSG_DATA(cmptr)) = sendfd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;

/*    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;*/

    iov[0].iov_base = "";
    iov[0].iov_len = 1;

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    return sendmsg(fd, &msg, 0);

}
ssize_t jmm_recv_fd(int fd,/* void* ptr, size_t nbytes, */int* recvfd)
{
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t n;

    *recvfd = -1;

    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }control_un;
    struct cmsghdr* cmptr;

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    char c;
    iov[0].iov_base = &c;
    iov[0].iov_len = 1;

    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if((n = recvmsg(fd, &msg, 0)) <= 0){
        return n;
    }

    if((cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
            cmptr->cmsg_len == CMSG_LEN(sizeof(int))){
        if(cmptr->cmsg_level != SOL_SOCKET){
            return -1;
        }
        if(cmptr->cmsg_type != SCM_RIGHTS){
            return -1;
        }
        *recvfd = *((int*)CMSG_DATA(cmptr));
    }else{
        *recvfd = -1;
    }
    return n;
}






#ifdef DEBUG
void jmm_print_space(int ls)
{
    int i=0;
    for(i=0; i<ls; i++){
        printf(" ");
    }
}
#endif
