#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "common.h"

int main(void) {
    lcl_init_filters();
    int result = 0;

    result |= test_app_filter_seq();
    result |= test_app_filter();
    result |= test_conv_array();

    lcl_free_filters();
    return result;
}