#include "common.h"

int main(void) {
    lcl_init_filters();
    int result = 0;

    result |= test_app_filter_seq();
    result |= test_app_filter();

    lcl_free_filters();
    return result;
}