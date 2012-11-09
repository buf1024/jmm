/*
 * list.c
 *
 *  Created on: 2012-11-9
 *      Author: buf1024@gmail.com
 */

#include <stdlib.h>
#include "list.h"

static list_node* __list_alloc_node(void* value, list_alloc_t alloc_fuc)
{
    list_node* node = (list_node*) malloc(sizeof(*node));
    if (node) {
        node->next_node = NULL;
        node->pre_node = NULL;
        if (alloc_fuc) {
            node->value = alloc_fuc(value);
        }else{
            node->value = value;
        }
    }
    return node;
}

static void __list_free_node(list_node* node, list_free_t free_func)
{
    if(node){
        if(free_func){
            free_func(node->value);
        }
        free(node);
    }
}

list* list_set_attr_cmp(list* list, list_cmp_t cmp)
{
    if(list){
        list->cmp = cmp;
    }
    return list;
}
list_cmp_t list_get_attr_cmp(list* list)
{
    if(list){
        return list->cmp;
    }
    return NULL;
}
list* list_set_attr_alloc(list* list, list_alloc_t alloc)
{
    if(list){
        list->alloc = alloc;
    }
    return list;
}
list_alloc_t list_get_attr_alloc(list* list)
{
    if(list){
        return list->alloc;
    }
    return NULL;
}
list* list_set_attr_free(list* list, list_free_t free)
{
    if(list){
        list->free = free;
    }
    return list;
}
list_free_t list_get_attr_free(list* list)
{
    if(list){
        return list->free;
    }
    return NULL;
}

unsigned list_len(list* list)
{
    if(list){
        return list->len;
    }
    return 0;
}

list* list_create()
{
    list* list;

    list = (struct list*)malloc(sizeof(struct list));

    if (list) {

        list->alloc = NULL;
        list->cmp = NULL;
        list->free = NULL;
        list->head = NULL;
        list->tail = NULL;
        list->len = 0;
    }

    return list;
}
void  list_destroy(list* list)
{
    if(list){
        list_node* node = list->head;
        list_node* pre  = NULL;
        while(node){
            pre = node;
            node = node->next_node;

            __list_free_node(pre, list->free);
        }
        free(list);
    }
}

list* list_add_head(list* list, void* value)
{
    if(list){
        list_node* node = __list_alloc_node(value, list->alloc);
        if(node){
            list_node* pre_head = list->head;

            if (pre_head) {
                list->head = node;
                node->next_node = pre_head;
                pre_head->pre_node = node;
            }else{
                list->head = node;
                list->tail = node;
            }
            list->len++;
        }
    }
    return list;
}
list* list_add_tail(list* list, void* value)
{
    if(list){
        list_node* node = __list_alloc_node(value, list->alloc);
        if(node){
            list_node* pre_tail = list->tail;
            if(pre_tail){
                list->tail = node;
                node->pre_node = pre_tail;
                pre_tail->next_node = node;
            }else{
                list->tail = node;
                list->head = node;
            }
            list->len++;
        }
    }
    return list;
}
list* list_add_before(list* list, list_node* node, void* value)
{
    if(list && node){
        list_node* item = __list_alloc_node(value, list->alloc);
        if(item){
            if(node->pre_node){
                node->pre_node->next_node = item;
                item->next_node = node;
                item->pre_node = node->pre_node;
                node->pre_node = item;
            }else{
                item->next_node = node;
                node->pre_node = item;
                list->head = item;
            }
            list->len++;
        }
    }
    return list;
}
list* list_add_after(list* list, list_node* node, void* value)
{
    if(list && node){
        list_node* item = __list_alloc_node(value, list->alloc);
        if(item){
            if(node->next_node){
                node->next_node->pre_node = item;
                item->next_node = node->next_node;
                item->pre_node = node;
                node->next_node = item;
            }else{
                node->next_node = item;
                item->pre_node = node;
                list->tail = item;
            }
            list->len++;
        }
    }
    return list;
}

list* list_add(list* list, void* value)
{
    if(list){
        if(list->cmp){
            list_iter* iter = list_iter_create(list, 1);
            if(iter){
                list_node* node = NULL;
                while((node = list_iter_next(iter)) != NULL){
                    if(list->cmp(node->value, value) > 0){
                        break;
                    }
                }
                if(node){
                    list = list_add_before(list, node, value);
                }else{
                    list = list_add_tail(list, value);
                }
                list_iter_destroy(iter);
            }
        }else{
            list = list_add_tail(list, value);
        }
    }
    return list;
}

list* list_del(list* list, void* value)
{
    if(list){
        list_node* node = list_find(list, value);
        if(node){
            if(node->pre_node){
                node->pre_node->next_node = node->next_node;
            }else{
                list->head = node->next_node;
                if(node->next_node){
                    node->next_node->pre_node = NULL;
                }
            }
            if(node->next_node){
                node->next_node->pre_node =node->pre_node;
            }else{
                list->tail = node->pre_node;
                if(node->pre_node){
                    node->pre_node->next_node = NULL;
                }
            }
            __list_free_node(node, list->free);
            list->len--;
        }
    }
    return NULL;
}
list* list_del_head(list* list)
{
    if(list){
        list_node* node = NULL;
        if(list->head){
            node = list->head;
            list->head = node->next_node;
            if(list->head){
                list->head->pre_node = NULL;
            }
        }
        if(node){
            __list_free_node(node, list->free);
        }
    }
    return list;
}
list* list_del_tail(list* list)
{
    if(list){
        list_node* node = NULL;
        if(list->tail){
            node = list->tail;
            list->tail = list->tail->pre_node;
            if(list->tail){
                list->tail->next_node = NULL;
            }
        }
        if(node){
            __list_free_node(node, list->free);
        }
    }
    return list;
}

list_node* list_get_head(list* list)
{
    if(list){
        return list->head;
    }
    return NULL;
}
list_node* list_get_tail(list* list)
{
    if(list){
        return list->tail;
    }
    return NULL;
}

list_node* list_find(list* list, void* value)
{
    list_node* node = NULL;
    if(list){
        list_iter* iter = list_iter_create(list, 1);
        if(iter){
            while((node = list_iter_next(iter)) != NULL){
                if(list->cmp){
                    if(list->cmp(node->value, value) == 0){
                        break;
                    }
                }else{
                    if(node->value == value){
                        break;
                    }
                }
            }
            list_iter_destroy(iter);
        }
    }
    return node;
}

list_iter* list_iter_create(list* list, int direct)
{
    list_iter* iter = NULL;
    if(list){
        iter = (list_iter*)malloc(sizeof(*iter));
        if(iter){
            iter->list = list;
            iter->direct = direct;
            iter->node = NULL;
            iter->done = 0;
        }
    }
    return iter;
}
void       list_iter_destroy(list_iter* iter)
{
    if(iter){
        free(iter);
    }
}
list_iter* list_iter_reset(list_iter* iter)
{
    if (iter) {
        iter->node = NULL;
        iter->done = 0;
    }
    return iter;
}
list_node* list_iter_next(list_iter* iter)
{
    list_node* node = NULL;
    if (iter) {
        if (!iter->done) {
            if (iter->direct >= 0) {
                if (!iter->node) {
                    iter->node = iter->list->head;
                }
                if (iter->node) {
                    node = iter->node->next_node;
                    iter->node = node;
                }
            } else {
                if (!iter->node) {
                    iter->node = iter->list->tail;
                }
                if (iter->node) {
                    node = iter->node->pre_node;
                    iter->node = node;
                }
            }

            if (!iter->node) {
                iter->done = 1;
            }
        }
    }
    return node;
}
