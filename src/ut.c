/*
 * ut.c
 *
 *  Created on: 2012-11-8
 *      Author: buf1024@gmail.com
 */

#include "test.h"

IMPL_G(simple);
DECL_R(suite) {G(simple), NULL};

#ifdef __UNIT_TEST__
int main(int argc, char **argv)
{

    INIT_TEST(argc, argv);
    RUN_ALL_TEST();
}
#endif
