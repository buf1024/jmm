/*
 * test.h
 *
 *  Created on: 2012-11-8
 *      Author: buf1024@gmail.com
 */
/*
 * gtest is an excellent unit test tool, but sometimes we don't want
 * its full functionality. so, I create a very simple test file
 * providing limited function using c++. but this is c source, using c
 * to implement the same functionality seems a little difficult.
 * Enhance, I give up, instead, I provide this ugly implementation for
 * c, it is very hard to use. who can help me to implement the similar
 * function using c with almost the same simple interface?
 */

#ifndef __48SLOTS_TEST_H__
#define __48SLOTS_TEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Routine to see if a text string is matched by a wildcard pattern.
 * Returns true if the text is matched, or false if it is not matched
 * or if the pattern is invalid.
 *  *        matches zero or more characters
 *  ?        matches a single character
 *  [abc]    matches 'a', 'b' or 'c'
 *  \c       quotes character c
 *  Adapted from code written by Ingo Wilken.
 */
static int __dummy_match(const char * text, const char * pattern)
{
    const char * retryPat;
    const char * retryText;
    int ch;
    int found;

    retryPat = NULL;
    retryText = NULL;

    while (*text || *pattern) {
        ch = *pattern++;

        switch (ch) {
        case '*':
            retryPat = pattern;
            retryText = text;
            break;

        case '[':
            found = 0;

            while ((ch = *pattern++) != ']') {
                if (ch == '\\')
                    ch = *pattern++;

                if (ch == '\0')
                    return 0;

                if (*text == ch)
                    found = 1;
            }

            if (!found) {
                pattern = retryPat;
                text = ++retryText;
            }

            if (*text++ == '\0')
                return 0;

            break;

        case '?':
            if (*text++ == '\0')
                return 0;

            break;

        case '\\':
            ch = *pattern++;

            if (ch == '\0')
                return 0;

            if (*text == ch) {
                if (*text)
                    text++;
                break;
            }

            if (*text) {
                pattern = retryPat;
                text = ++retryText;
                break;
            }

            return 0;

        default:
            if (*text == ch) {
                if (*text)
                    text++;
                break;
            }

            if (*text) {
                pattern = retryPat;
                text = ++retryText;
                break;
            }

            return 0;
        }

        if (pattern == NULL )
            return 0;
    }

    return 1;
}

static char __dummy_get_hex(char ch)
{
    ch = (0x0F & ch);
    if (ch >= 10) {
        return 'A' + ch - 10;
    }
    return '0' + ch;
}
/*remember to free after successfully cal the function.*/
static char* __dummy_hex_dump(void* buf, int size)
{
    char* rs_buf = NULL;
    if (buf && size > 0) {
        rs_buf = (char*) malloc(size * 2);
        if (rs_buf == NULL )
            return rs_buf;

        const char* tmp_buf = (const char*) buf;
        int tmp_size = 0;
        while (tmp_size < size) {
            char ch = *(tmp_buf + tmp_size);
            rs_buf[tmp_size * 2] = __dummy_get_hex((char) ((0xF0 & ch) >> 4));
            rs_buf[tmp_size * 2 + 1] = __dummy_get_hex((char) (0x0F & ch));
            tmp_size++;
        }
    }
    return rs_buf;
}
/////////////////////////////////////////////////////////////////////////////////////////
typedef void (*__dummy_test_case_func_t)();

typedef struct __dummy_test_case_t{
    char fxt_name[256];
    char  tc_name[256];
    __dummy_test_case_func_t func;
}__dummy_test_case_t;

typedef struct __dummy_group_test_t{
    __dummy_test_case_t* tc;
    struct __dummy_group_test_t* next_tc;
}__dummy_group_test_t;

typedef struct __dummy_test_groups_t{
    char fxt_name[256];
    __dummy_group_test_t* gtc;
    struct __dummy_suite_test_t* next_gtc;
}__dummy_group_tests_t;


static __dummy_group_tests_t* __dummy_the_test = NULL;
static char* __dummy_filter_pattern            = "*";
//0 invalid 1 help 2 list 3 filter/run
static int __dummy_run_opt                     = 3;

static void __dummy_help()
{
    printf(
            "Very Very Simple Test Framework Help Message\n\n"
             "--help\n"
             "   Print this help messages\n\n"
             "--list_tests\n"
             "   List the names of all tests instead of running them. The name of\n"
             "   TEST(Foo, Bar) is \"Foo.Bar\".\n\n"
             "--filter=POSTIVE_PATTERNS[-NEGATIVE_PATTERNS]\n"
             "   Run only the tests whose name matches one of the positive patterns but\n"
             "   none of the negative patterns. '?' matches any single character; '*'\n"
             "   matches any substring; ':' separates two patterns.\n");
}

static void __dummy_sort_test(__dummy_test_case_t*** test)
{
    __dummy_test_case_t** gtc = NULL;
    __dummy_test_case_t*  tc;

    int i = 0;

    __dummy_group_tests_t* grps = __dummy_the_test;
    __dummy_group_tests_t* cur_grp = grps;
    __dummy_group_tests_t* pre_grp = NULL;

    while ((gtc = test[i++]) != NULL ) {
        int j = 0;
        while ((tc = gtc[j++]) != NULL ) {
            printf("TF: %s, TC: %s\n", tc->fxt_name, tc->tc_name);
            cur_grp = grps;
            if (cur_grp == NULL ) {
                __dummy_group_tests_t* g_item = (__dummy_group_tests_t*)malloc(sizeof(*g_item));

                g_item->gtc = NULL;
                g_item->next_gtc = NULL;
                memset(g_item->fxt_name, 0, sizeof(g_item->fxt_name));

                __dummy_the_test = g_item;
                cur_grp = g_item;

            }else{
                while(cur_grp){
                    int cmp = strcmp(cur_grp->fxt_name, tc->fxt_name);
                    if(cmp != 0){
                        pre_grp = cur_grp;
                        cur_grp = cur_grp->next_gtc;
                    }else{
                        break;
                    }
                }
                if(!cur_grp){
                    __dummy_group_tests_t* g_item = (__dummy_group_tests_t*)malloc(sizeof(*g_item));

                    g_item->gtc = NULL;
                    g_item->next_gtc = NULL;
                    memset(g_item->fxt_name, 0, sizeof(g_item->fxt_name));

                    pre_grp->next_gtc = g_item;
                    cur_grp = g_item;
                }
            }

            __dummy_test_case_t* tc_item = (__dummy_test_case_t*)malloc(sizeof(*tc_item));
            strcpy(tc_item->fxt_name, tc->fxt_name);
            strcpy(tc_item->tc_name, tc->tc_name);
            tc_item->func = tc->func;

            while(){

            }
        }
    }

}

static int __dummy_analyse_args(int argc, char** argv)
{
    int i = 0;
    int opt = 3; //0 invalid 1 help 2 list 3 filter/run

    for (; i < argc; i++) {
        char* p = strstr(argv[i], "--help");
        if (p) {
            opt = 1;
            continue;
        }
        p = strstr(argv[i], "--list_tests");
        if (p) {
            opt = 2;
            continue;
        }
        p = strstr(argv[i], "--filter");
        if (p) {
            opt = 3;
            p = strstr(p, "=");
            if (p) {
                int len = strlen(argv[i]);
                len = len - (p - argv[i]) + 1;
                p++;
                while(len--){
                    if(*p == ' '){
                        p++;
                    }
                }
                if(len == 0){
                    opt = 0;
                    continue;
                }
                __dummy_filter_pattern = p;
                continue;
            }
            opt = 0;
            continue;
        }
        opt = 0;
    }
    __dummy_run_opt = opt;
    return opt;
}

static int __dummy_run_test()
{
    return 0;
}
static int __dummy_list_test()
{
    return 0;
}

static int __dummy_init_test(int argc, char** argv, \
        __dummy_test_case_t*** test)
{
    __dummy_analyse_args(argc, argv);
    __dummy_sort_test(test);
    return 0;
}

static int __dummy_run_all_test()
{
    switch(__dummy_run_opt){
    case 0: //invalid
        printf("Invalid Option! Please reference the following help message\n\n");
        __dummy_help();
        break;
    case 1: //help
        __dummy_help();
        break;
    case 2: //filter
        __dummy_list_test();
        break;
    case 3: //run/filter
        printf("XXXXXXXX!!!PLEASE NOTE!!!XXXXXXXX ==> --filter=%s\n", __dummy_filter_pattern);
        __dummy_run_test();
        break;
    }
    return 0;
}

static int __dummy_cleanup_test()
{
    return 0;
}

static int __dummy_add_fail_test(const char* func_name)
{
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
#define TEST(tf, tc)                                                          \
    static void __dummy__##tf##__##tc##__func();                              \
    static __dummy_test_case_t __dummy__##tf##__##tc##__ =                    \
    {#tf, #tc, __dummy__##tf##__##tc##__func};                                \
    static void __dummy__##tf##__##tc##__func()                               \

#define DECL_G(test)                                                          \
    __dummy_test_case_t* __dummy__group__##test##__ [] =                      \

#define T(tf, tc)                                                             \
        &__dummy__##tf##__##tc##__                                            \

#define IMPL_G(test)                                                          \
    extern __dummy_test_case_t __dummy__group__##test##__;                    \

#define DECL_R(suite)                                                         \
        __dummy_test_case_t** __dummy__all_test__ [] =

#define G(test)   &__dummy__group__##test##__

///////////////////////////////////////////////////////////////////////////////
#define INIT_TEST(argc, argv)                                                 \
    __dummy_init_test(argc, argv, __dummy__all_test__)                        \

#define RUN_ALL_TEST()                                                        \
    int __dummy_dummy_ret = __dummy_run_all_test();                           \
    __dummy_cleanup_test();                                                   \
    return __dummy_dummy_ret;                                                 \

///////////////////////////////////////////////////////////////////////////////
#define EXPECT_EQ(a, b) {                                                     \
    if(a != b) {                                                              \
        printf("[W]File: %d  Line: %d EXPECT: %lld ACTUAL: %lld\n",           \
             __FILE__, __LINE__, (long long)a, (long long)b);                 \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define EXPECT_NEQ(a, b) {                                                    \
    if(a == b) {  \
        printf("[W]File: %d  Line: %d "                                       \
            "EXPECT: %lld != %lld ACTUAL: %lld == %lld\n",                    \
            __FILE__, __LINE__,                                               \
            (long long)a, (long long)b, (long long)a, (long long)b);          \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define EXPECT_STREQ(a, b) {                                                  \
    if(strncmp(a, b, strlen(a)) != 0) {                                       \
        printf("[W]File: %d  Line: %d EXPECT: %s ACTUAL: %s\n",               \
                __FILE__,__LINE__, a, b);                                     \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define EXPECT_STRNEQ(a, b) {                                                 \
    if(strncmp(a, b, strlen(a)) == 0) {                                       \
        printf("[W]File: %d  Line: %d EXPECT: %s != %s ACTUAL: %s == %s\n",   \
            __FILE__, __LINE__, a, b, a, b);                                  \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define EXPECT_BINEQ(a, b, s) {                                               \
    if(memcmp(a, b, s) != 0) {                                                \
        char* buf_a = __dummy_hex_dump(a, s);                                 \
        if(!buf_a){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            return;                                                           \
        }                                                                     \
        char* buf_b = __dummy_hex_dump(b, s);                                 \
        if(!buf_b){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            free(buf_a);                                                      \
            return;                                                           \
        }                                                                     \
        printf("[W]File: %d  Line: %d EXPECT: 0X%s ACTUAL: 0X%s\n",           \
            __FILE__, __LINE__, buf_a, buf_b);                                \
        free(buf_a);                                                          \
        free(buf_b);                                                          \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define EXPECT_BINNEQ(a, b, s) {                                              \
    if(memcmp(a, b, s) == 0) {                                                \
        char* buf_a = __dummy_hex_dump(a, s);                                 \
        if(!buf_a){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            return;                                                           \
        }                                                                     \
        char* buf_b = __dummy_hex_dump(b, s);                                 \
        if(!buf_b){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            free(buf_a);                                                      \
            return;                                                           \
        }                                                                     \
        printf("[W]File: %d  Line: %d "                                       \
            "EXPECT: 0X%s != 0X%s ACTUAL: 0X%s == 0X%s\n",                    \
            __FILE__, __LINE__, buf_a, buf_b, buf_a, buf_b);                  \
        free(buf_a);                                                          \
        free(buf_b);                                                          \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define EXPECT_TRUE(a) {                                                      \
    if(!((int)a)){                                                            \
        printf("[W]File: %d  Line: %d EXPECT: TRUE ACTUAL: FALSE\n",          \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define EXPECT_FALSE(a) {                                                     \
    if(!!((int)a)){                                                           \
        printf("[W]File: %d  Line: %d EXPECT: FALSE ACTUAL: TRUE\n",          \
        __dummy_add_fail_test(__func__);                                      \
    }                                                                         \
}                                                                             \

#define ASSERT_EQ(a, b) {                                                     \
    if(a != b) {                                                              \
        printf("[W]File: %d  Line: %d EXPECT: %lld ACTUAL: %lld\n",           \
            __FILE__, __LINE__, (long long)a, (long long)b);                  \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#define ASSERT_NEQ(a, b) {                                                    \
    if(a == b) {                                                              \
        printf("[W]File: %d  Line: %d "                                       \
            "EXPECT: %lld != %lld ACTUAL: %lld == %lld\n",                    \
            __FILE__, __LINE__,                                               \
            (long long)a, (long long)b, (long long)a, (long long)b);          \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#define ASSERT_STREQ(a, b) {                                                  \
    if(strncmp(a, b, strlen(a)) != 0) {                                       \
        printf("[W]File: %d  Line: %d EXPECT: %s ACTUAL: %s\n",               \
            __FILE__,__LINE__, a, b);                                         \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#define ASSERT_STRNEQ(a, b) {                                                 \
    if(strncmp(a, b, strlen(a)) == 0) {                                       \
        printf("[W]File: %d  Line: %d EXPECT: %s != %s ACTUAL: %s == %s\n",   \
            __FILE__, __LINE__, a, b, a, b);                                  \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#define ASSERT_BINEQ(a, b, s) {                                               \
    if(memcmp(a, b, s) != 0) {                                                \
        char* buf_a = __dummy_hex_dump(a, s);                                 \
        if(!buf_a){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            return;                                                           \
        }                                                                     \
        char* buf_b = __dummy_hex_dump(b, s);                                 \
        if(!buf_b){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            free(buf_a);                                                      \
            return;                                                           \
        }                                                                     \
        printf("[W]File: %d  Line: %d EXPECT: 0X%s ACTUAL: 0X%s\n",           \
            __FILE__, __LINE__, buf_a, buf_b);                                \
        free(buf_a);                                                          \
        free(buf_b);                                                          \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#define ASSERT_BINNEQ(a, b, s) {                                              \
    if(memcmp(a, b, s) == 0) {                                                \
        char* buf_a = __dummy_hex_dump(a, s);                                 \
        if(!buf_a){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            return;                                                           \
        }                                                                     \
        char* buf_b = __dummy_hex_dump(b, s);                                 \
        if(!buf_b){                                                           \
            printf("File:%s Line:%d, Fail to allocate memory\n");             \
            free(buf_a);                                                      \
            return;                                                           \
        }                                                                     \
        printf("[W]File: %d  Line: %d "                                       \
            "EXPECT: 0X%s != 0X%s ACTUAL: 0X%s == 0X%s\n",                    \
            __FILE__, __LINE__, buf_a, buf_b, buf_a, buf_b);                  \
        free(buf_a);                                                          \
        free(buf_b);                                                          \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#define ASSERT_TRUE(a) {                                                      \
    if(!((int)a)){                                                            \
        printf("[W]File: %d  Line: %d EXPECT: TRUE ACTUAL: FALSE\n",          \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#define ASSERT_FALSE(a) {                                                     \
    if(!!(int)a){                                                             \
        printf("[W]File: %d  Line: %d EXPECT: FALSE ACTUAL: TRUE\n",          \
        __dummy_add_fail_test(__func__);                                      \
        return;                                                               \
    }                                                                         \
}                                                                             \

#endif /* __48SLOTS_TEST_H__ */
