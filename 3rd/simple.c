/*
 * simple.c
 *
 *  Created on: 2012-11-8
 *      Author: buf1024@gmail.com
 */

#include "test.h"

TEST(a, b)
{
    printf("TEST(a, b)\n");
    __dummy_add_fail_test(__func__);

}
TEST(a, c)
{
    printf("TEST(a, c)\n");
    __dummy_add_fail_test(__func__);
}
TEST(C, c)
{
    printf("TEST(a, c)\n");
    __dummy_add_fail_test(__func__);
}

DECL_G(simple) {T(a, b), T(a, c), T(C, c), NULL};

