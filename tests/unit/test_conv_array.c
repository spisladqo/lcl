#include "common.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "common.h"

static lcl_queue_t read_queue;
static lcl_queue_t fore_queue;
static lcl_queue_t write_queue;

static pthread_mutex_t read_lock;
static pthread_mutex_t fore_lock;
static pthread_mutex_t write_lock;

static int fore_tasks_ready, write_tasks_ready;
static int read_done, fore_done, write_done;
static pthread_cond_t fore_cv, write_cv;

static void test_invalid_img_num(void **state) {
    assert_int_equal(lcl_conv_array(NULL, NULL, NULL, NULL, -1, (thread_jobs_t){0}), LCL_INVALID_ARGUMENT);
    assert_int_equal(lcl_conv_array(NULL, NULL, NULL, NULL, MAX_IMG_NUM + 1, (thread_jobs_t){0}), LCL_INVALID_ARGUMENT);
}

static void test_nullptrs(void **state) {
    char* src_paths[] = {"test1.bmp", "test2.bmp"};
    char* targ_paths[] = {"out1.bmp", "out2.bmp"};
    enum lcl_conv_mode modes[] = {pilewise, pilewise};
    lcl_filter_t* filters[] = {NULL, NULL};
    
    assert_int_equal(lcl_conv_array(NULL, targ_paths, modes, filters, 2, (thread_jobs_t){0}), LCL_INVALID_ARGUMENT);
    assert_int_equal(lcl_conv_array(src_paths, NULL, modes, filters, 2, (thread_jobs_t){0}), LCL_INVALID_ARGUMENT);
    assert_int_equal(lcl_conv_array(src_paths, targ_paths, NULL, filters, 2, (thread_jobs_t){0}), LCL_INVALID_ARGUMENT);
    assert_int_equal(lcl_conv_array(src_paths, targ_paths, modes, NULL, 2, (thread_jobs_t){0}), LCL_INVALID_ARGUMENT);
}

static void test_lcl_conv_array1(void **state) {
    char* src_paths[] = {"test.bmp"};
    char* targ_paths[] = {"out.bmp"};
    enum lcl_conv_mode modes[] = {pilewise};
    lcl_filter_t filter = id_filter;
    lcl_filter_t* filters[] = {&filter};
    
    thread_jobs_t jobs = {1, 1, 1, 1};
    assert_int_equal(lcl_conv_array(src_paths, targ_paths, modes, filters, 1, jobs), LCL_OK);

    remove("out.bmp");
}

static void test_lcl_conv_array2(void **state) {
    char* src_paths[] = {"test1.bmp", "test2.bmp"};
    char* targ_paths[] = {"out1.bmp", "out2.bmp"};
    enum lcl_conv_mode modes[] = {pilewise, pixelwise};
    lcl_filter_t filter1 = sharp_filter;
    lcl_filter_t filter2 = blur_filter;
    lcl_filter_t* filters[] = {&filter1, &filter2};
    
    thread_jobs_t jobs = {2, 2, 2, 2};
    assert_int_equal(lcl_conv_array(src_paths, targ_paths, modes, filters, 2, jobs), LCL_OK);

    remove("out1.bmp");
    remove("out2.bmp");
}

int test_conv_array(void) {
    const struct CMUnitTest test_conv_array[] = {
        cmocka_unit_test(test_invalid_img_num),
        cmocka_unit_test(test_nullptrs),
        cmocka_unit_test(test_lcl_conv_array1),
        cmocka_unit_test(test_lcl_conv_array2),
    };
    
    return cmocka_run_group_tests(test_conv_array, NULL, NULL);
}