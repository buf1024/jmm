/*
 * ciniconfig.c
 *
 *  Created on: 2012-5-10
 *      Author: buf1024@gmail.com
 */
#include "ciniconfig.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INI_READ_LINE (INI_MAX_NAME_LEN + INI_MAX_VALUE_LEN + 2)

//section
static int section_save(struct section* self, const char* file);
static int section_save_fd(struct section* self, int fd);
static int section_insert_item(struct section* self, const char* key, const char* value);
static int section_insert_string(struct section* self, const char* key, const char* value);
static int section_insert_char(struct section* self, const char* key, char ch);
static int section_insert_int(struct section* self, const char* key, int i);
static int section_insert_long(struct section* self, const char* key, long l);
static int section_insert_double(struct section* self, const char* key, double d);
static int section_delete_item(struct section* self, const char* key);
static struct item* section_get_item(struct section* self, const char* key);
static int section_get_string(struct section* self, const char* key, char* value, int size);
static int section_get_char(struct section* self, const char* key, char* ch);
static int section_get_int(struct section* self, const char* key, int* i);
static int section_get_long(struct section* self, const char* key, long* l);
static int section_get_double(struct section* self, const char* key, double* d);

//helper
static struct item* alloc_item(const char* name, const char* value);
static void free_item(struct item* item);
static struct section* alloc_section(const char* name);
static void free_section(struct section* sec);
static int init_section(struct section* section, const char* name);
static int read_line(int fd, char* buf, int size);

//ini_config
static int ini_load(struct ini_config* self, const char* file);
static int ini_load_fd(struct ini_config* self, int fd);
static int ini_save(struct ini_config* self, const char* file);
static int ini_save_fd(struct ini_config* self, int fd);
static struct section* ini_insert_section(struct ini_config* self, const char* name);
static int ini_delete_section(struct ini_config* self, const char* name);
static struct section* ini_get_section(struct ini_config* self, const char* name);


int init_ini(struct ini_config* ini)
{
    if(ini){
        memset(ini, 0, sizeof(struct ini_config));
        ini->load = ini_load;
        ini->load_fd = ini_load_fd;
        ini->save = ini_save;
        ini->save_fd = ini_save_fd;
        ini->insert_section = ini_insert_section;
        ini->delete_section = ini_delete_section;
        ini->get_section = ini_get_section;
    }
    return 0;
}
int uninit_ini(struct ini_config* ini)
{
    if(ini){
        struct section* cur_sec = ini->sec;
        struct section* pre_sec = 0;
        while(cur_sec){
            pre_sec = cur_sec;
            cur_sec = cur_sec->next;

            free_section(pre_sec);
        }
        memset(ini, 0, sizeof(struct ini_config));
    }
    return 0;
}
//section
static int section_save(struct section* self, const char* file)
{
    int ret = -1;
    if(self && file){
        int fd = open(file, O_RDWR | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if(fd != -1){
            ret = section_save_fd(self, fd);
            close(fd);
        }
    }
    return ret;
}
static int section_save_fd(struct section* self, int fd)
{
    int ret = 0;
    if (self && fd >= 0) {
        struct item* item = 0;
        if(self->name[0] != '\0'){
            write(fd, "[", 1);
            write(fd, self->name, strlen(self->name));
            write(fd, "]\n", 2);
            item = self->item;
            while(item){
                if(item->name[0] != '\0' && item->value[0] != '\0'){
                    write(fd, item->name, strlen(item->name));
                    write(fd, "=", 1);
                    write(fd, item->value, strlen(item->value));
                    write(fd, "\n", 1);
                }
                item = item->next;
            }
        }
    }
    return ret;
}
static int section_insert_item(struct section* self, const char* key, const char* value)
{
    int ret = -1;
    if(self && key && value){
        struct item* new_item = alloc_item(key, value);
        if(self->item == 0){
            self->item = new_item;
        }else{
            struct item* cur_item = self->item;
            struct item* pre_item = 0;
            int update_flag = 0;
            while (cur_item) {
                if (strcmp(cur_item->name, key) == 0) {
                    if (pre_item) {
                        self->item = new_item;
                    } else {
                        pre_item->next = new_item;
                        new_item->next = cur_item->next;
                    }
                    free_item(cur_item);
                    update_flag = 1;
                    break;
                }
                pre_item = cur_item;
                cur_item = cur_item->next;
            }
            if (!update_flag) {
                if (pre_item) {
                    pre_item->next = new_item;
                } else {
                    self->item = new_item;
                }
            }
        }
        ret = 0;


    }
    return ret;
}
static int section_insert_string(struct section* self, const char* key, const char* value)
{
    return section_insert_item(self, key, value);
}
static int section_insert_char(struct section* self, const char* key, char ch)
{
    char value[32] = {0};
    snprintf(value, 32, "%c", ch);
    return section_insert_item(self, key, value);
}
static int section_insert_int(struct section* self, const char* key, int i)
{
    char value[32] = {0};
    snprintf(value, 32, "%d", i);
    return section_insert_item(self, key, value);
}
static int section_insert_long(struct section* self, const char* key, long l)
{
    char value[32] = {0};
    snprintf(value, 32, "%ld", l);
    return section_insert_item(self, key, value);
}
static int section_insert_double(struct section* self, const char* key, double d)
{
    char value[32] = {0};
    snprintf(value, 32, "%lf", d);
    return section_insert_item(self, key, value);
}
static int section_delete_item(struct section* self, const char* key)
{
    if (self && key) {
        struct item* item = self->item;
        struct item* pre_item = 0;
        while (item) {
            if(strcmp(item->name, key) == 0){
                if(self->item == item){
                    free_item(item);
                    self->item = 0;
                }else{
                    pre_item->next = item->next;
                    free_item(item);
                }
                break;
            }
            pre_item = item;
            item = item->next;
        }
    }
    return 0;
}
static struct item* section_get_item(struct section* self, const char* key)
{
    struct item* item_ret = 0;
    if (self && key) {
        struct item* item = self->item;
        while (item) {
            if (strcmp(item->name, key) == 0) {
                item_ret = item;
                break;
            }
            item = item->next;
        }
    }
    return item_ret;
}
static int section_get_string(struct section* self, const char* key, char* value, int size)
{
    int ret = -1;
    struct item* item = section_get_item(self, key);
    if(item){
        if(strlen(item->name) < size){
            strncpy(value, item->value, size);
            ret = 0;
        }
    }
    return ret;
}
static int section_get_char(struct section* self, const char* key, char* ch)
{
    int ret = -1;
    struct item* item = section_get_item(self, key);
    if(item){
        *ch = *(item->value);
    }
    return ret;
}
static int section_get_int(struct section* self, const char* key, int* i)
{
    int ret = -1;
    struct item* item = section_get_item(self, key);
    if(item){
        sscanf(item->value, "%d", i);
        ret = 0;
    }
    return ret;
}
static int section_get_long(struct section* self, const char* key, long* l)
{
    int ret = -1;
    struct item* item = section_get_item(self, key);
    if(item){
        sscanf(item->value, "%ld", l);
        ret = 0;
    }
    return ret;
}
static int section_get_double(struct section* self, const char* key, double* d)
{
    int ret = -1;
    struct item* item = section_get_item(self, key);
    if(item){
        sscanf(item->value, "%lf", d);
        ret = 0;
    }
    return ret;
}

//helper
static struct item* alloc_item(const char* name, const char* value)
{
    struct item* item = 0;
    if (name && value) {
        item = (struct item*) malloc(sizeof(struct item));
        if (item) {
            memset(item, 0, sizeof(struct item));
            strncpy(item->name, name, INI_MAX_NAME_LEN);
            strncpy(item->value, value, INI_MAX_VALUE_LEN);
        }
    }
    return item;
}
static void free_item(struct item* item)
{
    if(item){
        free(item);
    }
}
static struct section* alloc_section(const char* name)
{
    struct section* sec = 0;
    if (name) {
        sec = (struct section*)malloc(sizeof(struct section));
        if (sec) {
            init_section(sec, name);

        }
    }
    return sec;
}
static void free_section(struct section* sec)
{
    if(sec){

        struct item* cur_item = sec->item;
        struct item* pre_item = 0;
        while(cur_item){
            pre_item = cur_item;
            cur_item = cur_item->next;

            free(pre_item);
        }

        free(sec);
    }
}
static int init_section(struct section* section, const char* name)
{
    if(section){
        memset(section, 0, sizeof(struct section));
        if(name){
            strncpy(section->name, name, INI_MAX_NAME_LEN);
        }

        section->save = section_save;
        section->save_fd = section_save_fd;
        section->insert_item = section_insert_item;
        section->insert_string = section_insert_string;
        section->insert_char = section_insert_char;
        section->insert_int = section_insert_int;
        section->insert_long = section_insert_long;
        section->insert_double = section_insert_double;
        section->delete_item = section_delete_item;
        section->get_item = section_get_item;
        section->get_string = section_get_string;
        section->get_char = section_get_char;
        section->get_int = section_get_int;
        section->get_long = section_get_long;
        section->get_double = section_get_double;
    }
    return 0;
}
static int read_line(int fd, char* buf, int size)
{
    int index = 0;
    int rd = 0;
    char ch;

    memset(buf, 0, size);

    while((rd = read(fd, &ch, 1)) > 0){
        if(ch != '\n'){
            if(index <= size){
                buf[index++] = ch;
            }else{
                return -1;
            }
        }else{
            break;
        }
    }
    if(index > 0){
        if(buf[index - 1] == '\r'){
            buf[index - 1] = 0;
            index--;
        }
    }
    if(rd <= 0){
        return -1;
    }

    return index;
}
//ini_config
static int ini_load(struct ini_config* self, const char* file)
{
    int ret = -1;
    if (self && file) {
        int fd = open(file, O_RDONLY);
        if (fd != -1) {
            ret = ini_load_fd(self, fd);
            close(fd);
        }
    }
    return ret;
}

static int ini_load_fd(struct ini_config* self, int fd)
{
    int ret = -1;
    if(self && fd >= 0){
        char line[INI_READ_LINE + 1] = "";
        char* tmp = 0;
        char name[INI_MAX_NAME_LEN + 1] = "0";
        char value[INI_MAX_VALUE_LEN + 1] = "0";

        struct section* cur_sec = 0;

        int len = 0;
        while(1){
            if((len = read_line(fd, line, INI_READ_LINE)) < 0){
                break;
            }
            if (*line == '#' || *line == ';' || len == 0)
            {
                continue;
            }
            // Start section
            memset(name, 0, INI_MAX_NAME_LEN + 1);
            memset(value, 0, INI_MAX_VALUE_LEN + 1);

            tmp = line;
            if (*tmp == '['){
                const char* name_del = ++tmp;
                while(*tmp != '\0' && *tmp != ']'){
                    tmp++;
                }
                if (*tmp == '\0'){
                    return -1;
                }
                strncpy(name, name_del, tmp - name_del);
                cur_sec = self->insert_section(self, name);
                continue;
            }
            if (cur_sec){
                const char* eq_del = tmp;
                while(*eq_del != '\0'){
                    if(*eq_del == '='){
                        break;
                    }
                    eq_del++;
                }
                if(*eq_del == '\0' || *(eq_del+1) == '\0'){
                    continue;
                }
                strncpy(name, tmp, eq_del - tmp);
                strcpy(value, eq_del + 1);
                cur_sec->insert_item(cur_sec, name, value);

            }
        }
        ret = 0;
    }
    return ret;
}
static int ini_save(struct ini_config* self, const char* file)
{
    int ret = -1;
    if(self && file){
        int fd = open(file, O_RDWR | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if(fd != -1){
            ret = ini_save_fd(self, fd);
            close(fd);
        }
    }
    return ret;
}
static int ini_save_fd(struct ini_config* self, int fd)
{
    int ret = 0;
    if (self && fd >= 0) {
        struct section* sec = self->sec;
        while (sec) {
            if(sec->save_fd == 0){
                ret = -1;
                break;
            }
            ret = sec->save_fd(sec, fd);
            if(ret != 0){
                break;
            }
            sec = sec->next;
        }
    }
    return ret;
}
static struct section* ini_insert_section(struct ini_config* self, const char* name)
{
    struct section* sec = 0;
    if(self){
        sec = alloc_section(name);
        if (sec) {
            struct section* cur_sec = self->sec;
            struct section* pre_sec = 0;
            int update = 0;
            while(cur_sec){
                if(strcmp(cur_sec->name, name) == 0){
                    if(pre_sec){
                        pre_sec->next = sec;
                        sec->next = cur_sec->next;
                    }else{
                        self->sec = sec;
                    }
                    free_section(cur_sec);
                    update = 1;
                    break;
                }
                pre_sec = cur_sec;
                cur_sec = cur_sec->next;
            }
            if(!update){
                if(pre_sec){
                    pre_sec->next = sec;
                }else{
                    self->sec = sec;
                }
            }
        }
    }
    return sec;
}
static int ini_delete_section(struct ini_config* self, const char* name)
{
    if(self && name){
        struct section* sec = self->sec;
        struct section* pre_sec = 0;
        while(sec){
            if(strcmp(name, sec->name) == 0){
                break;
            }
            pre_sec = sec;
            sec = sec->next;
        }
        if(sec){
            if(pre_sec == 0){
                self->sec = 0;
            }else{
                pre_sec->next = sec->next;
            }
            free_section(sec);
        }
    }
    return 0;
}
static struct section* ini_get_section(struct ini_config* self, const char* name)
{
    struct section* ret_sec = 0;
    if (self && name) {
        struct section* sec = self->sec;
        while (sec) {
            if (strcmp(name, sec->name) == 0) {
                ret_sec = sec;
                break;
            }
            sec = sec->next;
        }
    }
    return ret_sec;
}
