/*
 * jmm_event.h
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_JMM_EVENT_H__
#define __48SLOTS_JMM_EVENT_H__

// 控制进程事件
typedef struct jmm_event
{
    struct event_base* base;
    struct event* term;
    struct event* usr1;
    struct event* ctrl_c;
    struct event* child;

    struct evconnlistener* listener;

}jmm_event;

// 子进程事件
typedef struct jmm_event_wf
{
    struct event_base* base;
    struct event* term;
    struct event* usr1;
    struct event* ctrl_c;

    struct event* cmm;

    int sock_num;
    struct bufferevent** sock;

}jmm_event_wf;

// 控制进程事件
int jmm_init_event(struct event_base* base);
int jmm_uninit_event();

// 子进程事件
int jmm_init_event_wf(struct event_base* base);
int jmm_uninit_event_wf();

// 子进程socket事件
int jmm_init_event_sock(int idx, int sfd);


#endif /* __48SLOTS_JMM_EVENT_H__ */
