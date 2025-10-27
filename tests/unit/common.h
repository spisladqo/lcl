#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>

#include "../../src/lib/conv/common.h"
#include "../../src/lib/conv/queue.h"

bmp_img *create_test_image(int width, int height, bmp_pixel pixel);
lcl_filter_t *create_test_filter(int width, int height, double value);
void free_test_image(bmp_img *img);
void free_test_filter(lcl_filter_t *filter);

int test_app_filter(void);
int test_app_filter_seq(void);
int test_conv_array(void);
