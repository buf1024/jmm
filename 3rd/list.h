/*
 * list.h
 *
 *  Created on: 2012-11-9
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_LIST_H__
#define __48SLOTS_LIST_H__

typedef struct list_node {
    struct list_node* pre_node;
    struct list_node* next_node;
    void*             value;
}list_node;

typedef int (*list_cmp_t)(void*, void*);
typedef void* (*list_alloc_t)(void*);
typedef void (*list_free_t)(void*);

typedef struct list {
    list_node* head;
    list_node* tail;

    /*list attribute*/
    unsigned len;
    list_cmp_t cmp;
    list_alloc_t alloc;
    list_free_t free;
}list;

typedef struct list_iter {
    list* list;
    int direct;
    list_node* node;
    int done;
}list_iter;

list* list_set_attr_cmp(list* list, list_cmp_t cmp);
list_cmp_t list_get_attr_cmp(list* list);
list* list_set_attr_alloc(list* list, list_alloc_t alloc);
list_alloc_t list_get_attr_alloc(list* list);
list* list_set_attr_free(list* list, list_free_t free);
list_free_t list_get_attr_free(list* list);

unsigned list_len(list* list);

list* list_create();
void  list_destroy(list* list);

list* list_add_head(list* list, void* value);
list* list_add_tail(list* list, void* value);
list* list_add_before(list* list, list_node* node, void* value);
list* list_add_after(list* list, list_node* node, void* value);
list* list_add(list* list, void* value);

list* list_del(list* list, void* value);
list* list_del_head(list* list);
list* list_del_tail(list* list);

list_node* list_get_head(list* list);
list_node* list_get_tail(list* list);
list_node* list_find(list* list, void* value);

list_iter* list_iter_create(list* list, int direct);
void       list_iter_destroy(list_iter* iter);
list_iter* list_iter_reset(list_iter* iter);
list_node* list_iter_next(list_iter* iter);

#endif /* __48SLOTS_LIST_H__ */
