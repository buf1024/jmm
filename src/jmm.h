/*
 * jmm.h
 *
 *  Created on: 2012-6-8
 *      Author: buf1024@gmail.com
 */

#ifndef __48SLOTS_JMM_H__
#define __48SLOTS_JMM_H__

#include <sys/types.h>

#define REG_INIT_HOOK_FUN(fn)  prog_init_hook prog_init_hook_fn = fn;

typedef struct jmm_prog_in
{
    int len;
    char* bytes;
}jmm_prog_in;

typedef struct jmm_prog_out
{
    int status;
    int len;
    char* bytes;
}jmm_prog_out;

typedef struct jmm_hook
{
    const char* (*prog_name)();
    const char* (*prog_version)();
    const char* (*prog_desc)();

    int (*prog_init)(char*);
    int (*prog_service)(jmm_prog_in*, jmm_prog_out*);
    int (*prog_uninit)();

    void* (*prog_malloc)(size_t); // sys/types.h
    void (*prog_free)(void*);

}jmm_hook;

// helper
typedef void (*prog_init_hook)(jmm_hook* hook);


#endif /* __48SLOTS_JMM_H__ */
