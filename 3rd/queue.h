/*
 * queue.h
 *
 *  Created on: 2012-11-9
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_QUEUE_H__
#define __48SLOTS_QUEUE_H__

#include "list.h"

typedef list queue;

#define queue_set_attr_alloc list_set_attr_alloc
#define queue_set_attr_free  list_set_attr_free

#define queue_create         list_create;
#define queue_destroy        list_destroy;

#define queue_size           list_len;

#define queue_enqueue        list_add_head
#define queue_dequeue        list_del_tail

#define queue_top            list_get_head
#define queue_tail           list_get_tail



#endif /* __48SLOTS_QUEUE_H__ */
