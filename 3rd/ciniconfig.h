/*
 * ciniconfig.h
 *
 *  Created on: 2012-5-10
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_CINICONFIG_H__
#define __48SLOTS_CINICONFIG_H__

#define INI_MAX_NAME_LEN  255
#define INI_MAX_VALUE_LEN 255

// ini 配置值
struct item
{
    char name[INI_MAX_NAME_LEN + 1];
    char value[INI_MAX_VALUE_LEN + 1];

    struct item* next;
};

// ini 配置项
struct section
{
    char name[INI_MAX_NAME_LEN + 1];
    struct item* item;
    struct section* next;

    int (*save)(struct section* self, const char* file);
    int (*save_fd)(struct section* self, int fd);
    int (*insert_item)(struct section* self, const char* key, const char* value);
    int (*insert_string)(struct section* self, const char* key, const char* value);
    int (*insert_char)(struct section* self, const char* key, char ch);
    int (*insert_int)(struct section* self, const char* key, int i);
    int (*insert_long)(struct section* self, const char* key, long l);
    int (*insert_double)(struct section* self, const char* key, double d);
    int (*delete_item)(struct section* self, const char* key);
    struct item* (*get_item)(struct section* self, const char* key);
    int (*get_string)(struct section* self, const char* key, char* value, int size);
    int (*get_char)(struct section* self, const char* key, char* ch);
    int (*get_int)(struct section* self, const char* key, int* i);
    int (*get_long)(struct section* self, const char* key, long* l);
    int (*get_double)(struct section* self, const char* key, double* d);
};

// ini 配置文件
struct ini_config
{
    struct section* sec;

    int (*load)(struct ini_config* self, const char* file);
    int (*load_fd)(struct ini_config* self, int fd);
    int (*save)(struct ini_config* self, const char* file);
    int (*save_fd)(struct ini_config* self, int fd);
    struct section* (*insert_section)(struct ini_config* self, const char* name);
    int (*delete_section)(struct ini_config* self, const char* name);
    struct section* (*get_section)(struct ini_config* self, const char* name);
};

// 初始化回调
int init_ini(struct ini_config* ini);
// 释放资源
int uninit_ini(struct ini_config* ini);


#endif /* __48SLOTS_CINICONFIG_H__ */
