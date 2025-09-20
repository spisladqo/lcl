#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "../../lib/conv/common.h"

extern int test_app_filter_seq(void);

int main(void) {
    lcl_init_filters();
    int result = 0;
    
    result |= test_app_filter_seq();

    lcl_free_filters();
    return result;
}