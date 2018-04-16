#include <stdio.h>
#include "test_template.h"

static int v_test = 0;

void test()
{
    v_test = 100;
}

int main()
{
    test();

    TEST_ASSERT_EQUAL_INT(100, v_test);


    return 0;
}