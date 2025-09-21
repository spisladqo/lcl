#include "common.h"

static void test_nullptrs(void **state) {
    (void)state;

    bmp_img src, targ;
    lcl_filter_t filter;

    assert_int_equal(lcl_app_filter(pilewise, 1, NULL, NULL, NULL),
                     LCL_INVALID_ARGUMENT);
    assert_int_equal(lcl_app_filter(pilewise, 1, NULL, &src, &targ),
                     LCL_INVALID_ARGUMENT);
    assert_int_equal(lcl_app_filter(pilewise, 1, &filter, NULL, &targ),
                     LCL_INVALID_ARGUMENT);
    assert_int_equal(lcl_app_filter(pilewise, 1, &filter, &src, NULL),
                     LCL_INVALID_ARGUMENT);
}

static void test_diff_sizes(void **state) {
    (void)state;

    lcl_filter_t filter = id_filter;

    bmp_img src, targ;
    src.img_header.biWidth = 10;
    src.img_header.biHeight = 10;

    targ.img_header.biWidth = 5;
    targ.img_header.biHeight = 10;

    assert_int_equal(lcl_app_filter(pilewise, 1, &filter, &src, &targ),
                     LCL_SRC_TARG_DIFF_SIZES);
}

static void test_too_many_threads(void **state) {
    (void)state;

    lcl_filter_t filter = id_filter;
    bmp_img src, targ;
    src.img_header.biWidth = 10;
    src.img_header.biHeight = 10;
    targ.img_header.biWidth = 10;
    targ.img_header.biHeight = 10;

    assert_int_equal(
        lcl_app_filter(pilewise, MAX_THREADS + 1, &filter, &src, &targ),
        LCL_INVALID_ARGUMENT);
}

static void test_id_filter(void **state) {
    (void)state;

    lcl_filter_t *filter = create_test_filter(1, 1, 1.0);
    bmp_pixel test_pixel = {.red = 100, .green = 150, .blue = 200};
    bmp_img *src = create_test_image(4, 4, test_pixel);
    bmp_img *targ = create_test_image(4, 4, (bmp_pixel){0, 0, 0});

    int result = lcl_app_filter(pilewise, 2, filter, src, targ);
    assert_int_equal(result, LCL_OK);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            assert_int_equal(targ->img_pixels[y][x].red, 100);
            assert_int_equal(targ->img_pixels[y][x].green, 150);
            assert_int_equal(targ->img_pixels[y][x].blue, 200);
        }
    }

    free_test_filter(filter);
    free_test_image(src);
    free_test_image(targ);
}

static void test_zero_filter(void **state) {
    (void)state;

    lcl_filter_t *filter = create_test_filter(3, 3, 0.0);
    bmp_pixel test_pixel = {100, 150, 200};
    bmp_img *src = create_test_image(3, 3, test_pixel);
    bmp_img *targ = create_test_image(3, 3, (bmp_pixel){255, 255, 255});

    int result = lcl_app_filter(pixelwise, 2, filter, src, targ);
    assert_int_equal(result, LCL_OK);

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            assert_int_equal(targ->img_pixels[y][x].red, 0);
            assert_int_equal(targ->img_pixels[y][x].green, 0);
            assert_int_equal(targ->img_pixels[y][x].blue, 0);
        }
    }

    free_test_filter(filter);
    free_test_image(src);
    free_test_image(targ);
}

static void test_edges(void **state) {
    (void)state;

    lcl_filter_t *filter = create_test_filter(3, 3, 1.0 / 9.0);

    bmp_img *src = create_test_image(2, 2, (bmp_pixel){0, 0, 0});
    src->img_pixels[0][0] = (bmp_pixel){255, 0, 0};
    src->img_pixels[0][1] = (bmp_pixel){0, 255, 0};
    src->img_pixels[1][0] = (bmp_pixel){0, 0, 255};
    src->img_pixels[1][1] = (bmp_pixel){255, 255, 0};

    bmp_img *targ = create_test_image(2, 2, (bmp_pixel){0, 0, 0});

    int result = lcl_app_filter(pilewise, 1, filter, src, targ);
    assert_int_equal(result, LCL_OK);

    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 2; x++) {
            assert_true(targ->img_pixels[y][x].red >= 0 &&
                        targ->img_pixels[y][x].red <= 255);
            assert_true(targ->img_pixels[y][x].green >= 0 &&
                        targ->img_pixels[y][x].green <= 255);
            assert_true(targ->img_pixels[y][x].blue >= 0 &&
                        targ->img_pixels[y][x].blue <= 255);
        }
    }

    free_test_filter(filter);
    free_test_image(src);
    free_test_image(targ);
}

static void test_diff_modes(void **state) {
    (void)state;

    lcl_filter_t *filter = create_test_filter(1, 1, 1.0);
    bmp_pixel test_pixel = {100, 150, 200};
    bmp_img *src = create_test_image(3, 3, test_pixel);
    bmp_img *targ1 = create_test_image(3, 3, (bmp_pixel){0, 0, 0});
    bmp_img *targ2 = create_test_image(3, 3, (bmp_pixel){0, 0, 0});
    bmp_img *targ3 = create_test_image(3, 3, (bmp_pixel){0, 0, 0});

    assert_int_equal(lcl_app_filter(pilewise, 1, filter, src, targ1), LCL_OK);
    assert_int_equal(lcl_app_filter(pixelwise, 1, filter, src, targ2), LCL_OK);
    assert_int_equal(lcl_app_filter(rowwise, 1, filter, src, targ3), LCL_OK);

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            assert_int_equal(targ1->img_pixels[y][x].red,
                             targ2->img_pixels[y][x].red);
            assert_int_equal(targ1->img_pixels[y][x].green,
                             targ2->img_pixels[y][x].green);
            assert_int_equal(targ1->img_pixels[y][x].blue,
                             targ2->img_pixels[y][x].blue);

            assert_int_equal(targ2->img_pixels[y][x].red,
                             targ3->img_pixels[y][x].red);
            assert_int_equal(targ2->img_pixels[y][x].green,
                             targ3->img_pixels[y][x].green);
            assert_int_equal(targ2->img_pixels[y][x].blue,
                             targ3->img_pixels[y][x].blue);
        }
    }

    free_test_filter(filter);
    free_test_image(src);
    free_test_image(targ1);
    free_test_image(targ2);
    free_test_image(targ3);
}

int test_app_filter(void) {
    const struct CMUnitTest test_app_filter[] = {
        cmocka_unit_test(test_nullptrs),
        cmocka_unit_test(test_diff_sizes),
        cmocka_unit_test(test_too_many_threads),
        cmocka_unit_test(test_id_filter),
        cmocka_unit_test(test_zero_filter),
        cmocka_unit_test(test_edges),
        cmocka_unit_test(test_diff_modes),
    };

    return cmocka_run_group_tests(test_app_filter, NULL, NULL);
}