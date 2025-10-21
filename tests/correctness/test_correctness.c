#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>
#include "../../src/lib/conv/common.h"

#define MODES_NUM 4
#define SIZE_MISMATCH -1
#define COLOR_MISMATCH -2

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

const enum lcl_conv_mode modes[] = {rowwise, columnwise, pilewise, pixelwise};
const lcl_filter_t *filt[] = {
    &id_filter,         &blur_filter,       &Blur_filter,     &BLUR_filter,
    &edge_filter,       &sharp_filter,      &negative_filter, &emboss_filter,
    &left_shift_filter, &right_shift_filter};
const char *srcs[] = {
    "images/input/Almond_van_Gogh.bmp", "images/input/Impression_Sunrise.bmp",
    "images/input/Rye.bmp", "images/input/Sunflowers_van_Gogh.bmp",
    "images/input/The_Ninth_Wave.bmp"};
const char *targs[] = {
    "images/output/Almond_van_Gogh.bmp", "images/output/Impression_Sunrise.bmp",
    "images/output/Rye.bmp", "images/output/Sunflowers_van_Gogh.bmp",
    "images/output/The_Ninth_Wave.bmp"};

int compare_imgs(const bmp_img *src, const bmp_img *targ) {
    int height = src->img_header.biHeight;
    int height_targ = targ->img_header.biHeight;
    int width = src->img_header.biWidth;
    int width_targ = targ->img_header.biWidth;

    if (height != height_targ) {
        printf("src and targ have different heights: src has %d, targ has %d\n",
               height, height_targ);
        return SIZE_MISMATCH;
    }
    if (width != width_targ) {
        printf("src and targ have different widths: src has %d, targ has %d\n",
               width, width_targ);
        return SIZE_MISMATCH;
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            bmp_pixel pixel1 = src->img_pixels[y][x];
            bmp_pixel pixel2 = targ->img_pixels[y][x];
            if (pixel1.red != pixel2.red || pixel1.green != pixel2.green ||
                pixel1.blue != pixel2.blue) {
                printf(
                    "pixels on (x:%d, y:%d) don't match:\n src has (r:%d, "
                    "g:%d, b:%d), targ has (r:%d, g:%d, b:%d)\n",
                    x, y, pixel1.red, pixel1.green, pixel1.blue, pixel2.red,
                    pixel2.green, pixel2.blue);
                return COLOR_MISMATCH;
            }
        }
    }
    return 0;
}

#define DEFINE_SEQ_ID_TEST(name, img_idx)                          \
    static void test_seq_id_##name(void **state) {                 \
        (void)state;                                               \
        const char *srcn = srcs[img_idx];                          \
        bmp_img src, targ;                                         \
        int err = bmp_img_read(&src, srcn);                        \
        if (err) {                                                 \
            printf("could not read img to src: error %d\n", err);  \
            fail();                                                \
            return;                                                \
        }                                                          \
        err = bmp_img_read(&targ, srcn);                           \
        if (err) {                                                 \
            printf("could not read img to targ: error %d\n", err); \
            fail();                                                \
            return;                                                \
        }                                                          \
        lcl_app_filter_seq(&id_filter, &src, &targ);               \
        err = compare_imgs(&src, &targ);                           \
        if (err) {                                                 \
            printf("sequential id test failed: %s\n", srcn);       \
            fail();                                                \
            return;                                                \
        }                                                          \
    }

#define DEFINE_SEQ_SHIFT_TEST(name, img_idx)                       \
    static void test_seq_shift_##name(void **state) {              \
        (void)state;                                               \
        const char *srcn = srcs[img_idx];                          \
        bmp_img src, temp, targ;                                   \
        int err = bmp_img_read(&src, srcn);                        \
        if (err) {                                                 \
            printf("could not read img to src: error %d\n", err);  \
            fail();                                                \
            return;                                                \
        }                                                          \
        err = bmp_img_read(&temp, srcn);                           \
        if (err) {                                                 \
            printf("could not read img to temp: error %d\n", err); \
            fail();                                                \
            return;                                                \
        }                                                          \
        err = bmp_img_read(&targ, srcn);                           \
        if (err) {                                                 \
            printf("could not read img to targ: error %d\n", err); \
            fail();                                                \
            return;                                                \
        }                                                          \
        lcl_app_filter_seq(&left_shift_filter, &src, &temp);       \
        lcl_app_filter_seq(&right_shift_filter, &temp, &targ);     \
        err = compare_imgs(&src, &targ);                           \
        if (err) {                                                 \
            printf("sequential shift test failed: %s\n", srcn);    \
            fail();                                                \
            return;                                                \
        }                                                          \
    }

#define DEFINE_PARAL_TEST(name, img_idx, filter_suffix, filter_idx)    \
    static void test_paral_##name##_##filter_suffix(void **state) {    \
        (void)state;                                                   \
        const char *srcn = srcs[img_idx];                              \
        lcl_filter_t *filter = (lcl_filter_t *)filt[filter_idx];       \
        bmp_img src, targ_seq, targ_par;                               \
        int err = bmp_img_read(&src, srcn);                            \
        if (err) {                                                     \
            printf("could not read img to src: error %d\n", err);      \
            fail();                                                    \
            return;                                                    \
        }                                                              \
        err = bmp_img_read(&targ_seq, srcn);                           \
        if (err) {                                                     \
            printf("could not read img to seq targ: error %d\n", err); \
            fail();                                                    \
            return;                                                    \
        }                                                              \
        err = bmp_img_read(&targ_par, srcn);                           \
        if (err) {                                                     \
            printf("could not read img to par targ: error %d\n", err); \
            fail();                                                    \
            return;                                                    \
        }                                                              \
        for (int i = 0; i < MODES_NUM; i++) {                          \
            lcl_app_filter_seq(filter, &src, &targ_seq);               \
            lcl_app_filter(modes[i], 8, filter, &src, &targ_par);      \
            err = compare_imgs(&targ_seq, &targ_par);                  \
            if (err) {                                                 \
                printf("parallel test failed: %s mode %d\n", srcn, i); \
                fail();                                                \
                return;                                                \
            }                                                          \
        }                                                              \
    }

#define DEFINE_PARQUEUE_TEST(name, img_idx, filter_suffix, filter_idx)     \
    static void test_parqueue_##name##_##filter_suffix(void **state) {     \
        (void)state;                                                       \
        const char *srcn = srcs[img_idx];                                  \
        const char *targn = targs[img_idx];                                \
        bmp_img src, targ_seq, targ_par;                                   \
        int err = bmp_img_read(&src, srcn);                                \
        if (err) {                                                         \
            printf("could not read img to src: error %d\n", err);          \
            fail();                                                        \
            return;                                                        \
        }                                                                  \
        err = bmp_img_read(&targ_seq, srcn);                               \
        if (err) {                                                         \
            printf("could not read img to seq targ: error %d\n", err);     \
            fail();                                                        \
            return;                                                        \
        }                                                                  \
        err = bmp_img_read(&targ_par, srcn);                               \
        if (err) {                                                         \
            printf("could not read img to par targ: error %d\n", err);     \
            fail();                                                        \
            return;                                                        \
        }                                                                  \
        lcl_filter_t *filter = (lcl_filter_t *)filt[filter_idx];           \
        lcl_filter_t *filters[1] = {filter};                               \
        thread_jobs_t jobs = {1, 1, 8, 1};                                 \
        char *srcns[] = {(char *)srcn};                                    \
        char *targns[] = {(char *)targn};                                  \
        for (int i = 0; i < MODES_NUM; i++) {                              \
            enum lcl_conv_mode conv[1] = {modes[i]};                       \
            lcl_app_filter_seq(filter, &src, &targ_seq);                   \
            bmp_img_write(&targ_seq, targn);                               \
            err = bmp_img_read(&targ_seq, targn);                          \
            if (err) {                                                     \
                printf("could not read img to seq targ: error %d\n", err); \
                fail();                                                    \
                return;                                                    \
            }                                                              \
            lcl_conv_array(srcns, targns, conv, filters, 1, jobs);         \
            err = bmp_img_read(&targ_par, targn);                          \
            if (err) {                                                     \
                printf("could not read img to par targ: error %d\n", err); \
                fail();                                                    \
                return;                                                    \
            }                                                              \
            err = compare_imgs(&targ_seq, &targ_par);                      \
            if (err) {                                                     \
                printf("parqueue test failed: %s mode %d\n", srcn, i);     \
                fail();                                                    \
                return;                                                    \
            }                                                              \
        }                                                                  \
    }

DEFINE_SEQ_ID_TEST(almond, 0)
DEFINE_SEQ_ID_TEST(sunrise, 1)
DEFINE_SEQ_ID_TEST(rye, 2)
DEFINE_SEQ_ID_TEST(sunflowers, 3)
DEFINE_SEQ_ID_TEST(ninth_wave, 4)

DEFINE_SEQ_SHIFT_TEST(almond, 0)
DEFINE_SEQ_SHIFT_TEST(sunrise, 1)
DEFINE_SEQ_SHIFT_TEST(rye, 2)
DEFINE_SEQ_SHIFT_TEST(sunflowers, 3)
DEFINE_SEQ_SHIFT_TEST(ninth_wave, 4)

DEFINE_PARAL_TEST(almond, 0, id, 0)
DEFINE_PARAL_TEST(almond, 0, blur, 1)
DEFINE_PARAL_TEST(almond, 0, Blur, 2)
DEFINE_PARAL_TEST(almond, 0, BLUR, 3)
DEFINE_PARAL_TEST(almond, 0, edge, 4)
DEFINE_PARAL_TEST(almond, 0, sharp, 5)
DEFINE_PARAL_TEST(almond, 0, negative, 6)
DEFINE_PARAL_TEST(almond, 0, emboss, 7)
DEFINE_PARAL_TEST(almond, 0, left_shift, 8)
DEFINE_PARAL_TEST(almond, 0, right_shift, 9)

DEFINE_PARAL_TEST(sunrise, 1, id, 0)
DEFINE_PARAL_TEST(sunrise, 1, blur, 1)
DEFINE_PARAL_TEST(sunrise, 1, Blur, 2)
DEFINE_PARAL_TEST(sunrise, 1, BLUR, 3)
DEFINE_PARAL_TEST(sunrise, 1, edge, 4)
DEFINE_PARAL_TEST(sunrise, 1, sharp, 5)
DEFINE_PARAL_TEST(sunrise, 1, negative, 6)
DEFINE_PARAL_TEST(sunrise, 1, emboss, 7)
DEFINE_PARAL_TEST(sunrise, 1, left_shift, 8)
DEFINE_PARAL_TEST(sunrise, 1, right_shift, 9)

DEFINE_PARAL_TEST(rye, 2, id, 0)
DEFINE_PARAL_TEST(rye, 2, blur, 1)
DEFINE_PARAL_TEST(rye, 2, Blur, 2)
DEFINE_PARAL_TEST(rye, 2, BLUR, 3)
DEFINE_PARAL_TEST(rye, 2, edge, 4)
DEFINE_PARAL_TEST(rye, 2, sharp, 5)
DEFINE_PARAL_TEST(rye, 2, negative, 6)
DEFINE_PARAL_TEST(rye, 2, emboss, 7)
DEFINE_PARAL_TEST(rye, 2, left_shift, 8)
DEFINE_PARAL_TEST(rye, 2, right_shift, 9)

DEFINE_PARAL_TEST(sunflowers, 3, id, 0)
DEFINE_PARAL_TEST(sunflowers, 3, blur, 1)
DEFINE_PARAL_TEST(sunflowers, 3, Blur, 2)
DEFINE_PARAL_TEST(sunflowers, 3, BLUR, 3)
DEFINE_PARAL_TEST(sunflowers, 3, edge, 4)
DEFINE_PARAL_TEST(sunflowers, 3, sharp, 5)
DEFINE_PARAL_TEST(sunflowers, 3, negative, 6)
DEFINE_PARAL_TEST(sunflowers, 3, emboss, 7)
DEFINE_PARAL_TEST(sunflowers, 3, left_shift, 8)
DEFINE_PARAL_TEST(sunflowers, 3, right_shift, 9)

DEFINE_PARAL_TEST(ninth_wave, 4, id, 0)
DEFINE_PARAL_TEST(ninth_wave, 4, blur, 1)
DEFINE_PARAL_TEST(ninth_wave, 4, Blur, 2)
DEFINE_PARAL_TEST(ninth_wave, 4, BLUR, 3)
DEFINE_PARAL_TEST(ninth_wave, 4, edge, 4)
DEFINE_PARAL_TEST(ninth_wave, 4, sharp, 5)
DEFINE_PARAL_TEST(ninth_wave, 4, negative, 6)
DEFINE_PARAL_TEST(ninth_wave, 4, emboss, 7)
DEFINE_PARAL_TEST(ninth_wave, 4, left_shift, 8)
DEFINE_PARAL_TEST(ninth_wave, 4, right_shift, 9)

DEFINE_PARQUEUE_TEST(almond, 0, id, 0)
DEFINE_PARQUEUE_TEST(almond, 0, blur, 1)
DEFINE_PARQUEUE_TEST(almond, 0, Blur, 2)
DEFINE_PARQUEUE_TEST(almond, 0, BLUR, 3)
DEFINE_PARQUEUE_TEST(almond, 0, edge, 4)
DEFINE_PARQUEUE_TEST(almond, 0, sharp, 5)
DEFINE_PARQUEUE_TEST(almond, 0, negative, 6)
DEFINE_PARQUEUE_TEST(almond, 0, emboss, 7)
DEFINE_PARQUEUE_TEST(almond, 0, left_shift, 8)
DEFINE_PARQUEUE_TEST(almond, 0, right_shift, 9)

DEFINE_PARQUEUE_TEST(sunrise, 1, id, 0)
DEFINE_PARQUEUE_TEST(sunrise, 1, blur, 1)
DEFINE_PARQUEUE_TEST(sunrise, 1, Blur, 2)
DEFINE_PARQUEUE_TEST(sunrise, 1, BLUR, 3)
DEFINE_PARQUEUE_TEST(sunrise, 1, edge, 4)
DEFINE_PARQUEUE_TEST(sunrise, 1, sharp, 5)
DEFINE_PARQUEUE_TEST(sunrise, 1, negative, 6)
DEFINE_PARQUEUE_TEST(sunrise, 1, emboss, 7)
DEFINE_PARQUEUE_TEST(sunrise, 1, left_shift, 8)
DEFINE_PARQUEUE_TEST(sunrise, 1, right_shift, 9)

DEFINE_PARQUEUE_TEST(rye, 2, id, 0)
DEFINE_PARQUEUE_TEST(rye, 2, blur, 1)
DEFINE_PARQUEUE_TEST(rye, 2, Blur, 2)
DEFINE_PARQUEUE_TEST(rye, 2, BLUR, 3)
DEFINE_PARQUEUE_TEST(rye, 2, edge, 4)
DEFINE_PARQUEUE_TEST(rye, 2, sharp, 5)
DEFINE_PARQUEUE_TEST(rye, 2, negative, 6)
DEFINE_PARQUEUE_TEST(rye, 2, emboss, 7)
DEFINE_PARQUEUE_TEST(rye, 2, left_shift, 8)
DEFINE_PARQUEUE_TEST(rye, 2, right_shift, 9)

DEFINE_PARQUEUE_TEST(sunflowers, 3, id, 0)
DEFINE_PARQUEUE_TEST(sunflowers, 3, blur, 1)
DEFINE_PARQUEUE_TEST(sunflowers, 3, Blur, 2)
DEFINE_PARQUEUE_TEST(sunflowers, 3, BLUR, 3)
DEFINE_PARQUEUE_TEST(sunflowers, 3, edge, 4)
DEFINE_PARQUEUE_TEST(sunflowers, 3, sharp, 5)
DEFINE_PARQUEUE_TEST(sunflowers, 3, negative, 6)
DEFINE_PARQUEUE_TEST(sunflowers, 3, emboss, 7)
DEFINE_PARQUEUE_TEST(sunflowers, 3, left_shift, 8)
DEFINE_PARQUEUE_TEST(sunflowers, 3, right_shift, 9)

DEFINE_PARQUEUE_TEST(ninth_wave, 4, id, 0)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, blur, 1)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, Blur, 2)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, BLUR, 3)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, edge, 4)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, sharp, 5)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, negative, 6)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, emboss, 7)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, left_shift, 8)
DEFINE_PARQUEUE_TEST(ninth_wave, 4, right_shift, 9)

int main(void) {
    lcl_init_filters();

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_seq_id_almond),
        cmocka_unit_test(test_seq_id_sunrise),
        cmocka_unit_test(test_seq_id_rye),
        cmocka_unit_test(test_seq_id_sunflowers),
        cmocka_unit_test(test_seq_id_ninth_wave),

        cmocka_unit_test(test_seq_shift_almond),
        cmocka_unit_test(test_seq_shift_sunrise),
        cmocka_unit_test(test_seq_shift_rye),
        cmocka_unit_test(test_seq_shift_sunflowers),
        cmocka_unit_test(test_seq_shift_ninth_wave),

        cmocka_unit_test(test_paral_almond_id),
        cmocka_unit_test(test_paral_almond_blur),
        cmocka_unit_test(test_paral_almond_Blur),
        cmocka_unit_test(test_paral_almond_BLUR),
        cmocka_unit_test(test_paral_almond_edge),
        cmocka_unit_test(test_paral_almond_sharp),
        cmocka_unit_test(test_paral_almond_negative),
        cmocka_unit_test(test_paral_almond_emboss),
        cmocka_unit_test(test_paral_almond_left_shift),
        cmocka_unit_test(test_paral_almond_right_shift),

        cmocka_unit_test(test_paral_sunrise_id),
        cmocka_unit_test(test_paral_sunrise_blur),
        cmocka_unit_test(test_paral_sunrise_Blur),
        cmocka_unit_test(test_paral_sunrise_BLUR),
        cmocka_unit_test(test_paral_sunrise_edge),
        cmocka_unit_test(test_paral_sunrise_sharp),
        cmocka_unit_test(test_paral_sunrise_negative),
        cmocka_unit_test(test_paral_sunrise_emboss),
        cmocka_unit_test(test_paral_sunrise_left_shift),
        cmocka_unit_test(test_paral_sunrise_right_shift),

        cmocka_unit_test(test_paral_rye_id),
        cmocka_unit_test(test_paral_rye_blur),
        cmocka_unit_test(test_paral_rye_Blur),
        cmocka_unit_test(test_paral_rye_BLUR),
        cmocka_unit_test(test_paral_rye_edge),
        cmocka_unit_test(test_paral_rye_sharp),
        cmocka_unit_test(test_paral_rye_negative),
        cmocka_unit_test(test_paral_rye_emboss),
        cmocka_unit_test(test_paral_rye_left_shift),
        cmocka_unit_test(test_paral_rye_right_shift),

        cmocka_unit_test(test_paral_sunflowers_id),
        cmocka_unit_test(test_paral_sunflowers_blur),
        cmocka_unit_test(test_paral_sunflowers_Blur),
        cmocka_unit_test(test_paral_sunflowers_BLUR),
        cmocka_unit_test(test_paral_sunflowers_edge),
        cmocka_unit_test(test_paral_sunflowers_sharp),
        cmocka_unit_test(test_paral_sunflowers_negative),
        cmocka_unit_test(test_paral_sunflowers_emboss),
        cmocka_unit_test(test_paral_sunflowers_left_shift),
        cmocka_unit_test(test_paral_sunflowers_right_shift),

        cmocka_unit_test(test_paral_ninth_wave_id),
        cmocka_unit_test(test_paral_ninth_wave_blur),
        cmocka_unit_test(test_paral_ninth_wave_Blur),
        cmocka_unit_test(test_paral_ninth_wave_BLUR),
        cmocka_unit_test(test_paral_ninth_wave_edge),
        cmocka_unit_test(test_paral_ninth_wave_sharp),
        cmocka_unit_test(test_paral_ninth_wave_negative),
        cmocka_unit_test(test_paral_ninth_wave_emboss),
        cmocka_unit_test(test_paral_ninth_wave_left_shift),
        cmocka_unit_test(test_paral_ninth_wave_right_shift),

        cmocka_unit_test(test_parqueue_almond_id),
        cmocka_unit_test(test_parqueue_almond_blur),
        cmocka_unit_test(test_parqueue_almond_Blur),
        cmocka_unit_test(test_parqueue_almond_BLUR),
        cmocka_unit_test(test_parqueue_almond_edge),
        cmocka_unit_test(test_parqueue_almond_sharp),
        cmocka_unit_test(test_parqueue_almond_negative),
        cmocka_unit_test(test_parqueue_almond_emboss),
        cmocka_unit_test(test_parqueue_almond_left_shift),
        cmocka_unit_test(test_parqueue_almond_right_shift),

        cmocka_unit_test(test_parqueue_sunrise_id),
        cmocka_unit_test(test_parqueue_sunrise_blur),
        cmocka_unit_test(test_parqueue_sunrise_Blur),
        cmocka_unit_test(test_parqueue_sunrise_BLUR),
        cmocka_unit_test(test_parqueue_sunrise_edge),
        cmocka_unit_test(test_parqueue_sunrise_sharp),
        cmocka_unit_test(test_parqueue_sunrise_negative),
        cmocka_unit_test(test_parqueue_sunrise_emboss),
        cmocka_unit_test(test_parqueue_sunrise_left_shift),
        cmocka_unit_test(test_parqueue_sunrise_right_shift),

        cmocka_unit_test(test_parqueue_rye_id),
        cmocka_unit_test(test_parqueue_rye_blur),
        cmocka_unit_test(test_parqueue_rye_Blur),
        cmocka_unit_test(test_parqueue_rye_BLUR),
        cmocka_unit_test(test_parqueue_rye_edge),
        cmocka_unit_test(test_parqueue_rye_sharp),
        cmocka_unit_test(test_parqueue_rye_negative),
        cmocka_unit_test(test_parqueue_rye_emboss),
        cmocka_unit_test(test_parqueue_rye_left_shift),
        cmocka_unit_test(test_parqueue_rye_right_shift),

        cmocka_unit_test(test_parqueue_sunflowers_id),
        cmocka_unit_test(test_parqueue_sunflowers_blur),
        cmocka_unit_test(test_parqueue_sunflowers_Blur),
        cmocka_unit_test(test_parqueue_sunflowers_BLUR),
        cmocka_unit_test(test_parqueue_sunflowers_edge),
        cmocka_unit_test(test_parqueue_sunflowers_sharp),
        cmocka_unit_test(test_parqueue_sunflowers_negative),
        cmocka_unit_test(test_parqueue_sunflowers_emboss),
        cmocka_unit_test(test_parqueue_sunflowers_left_shift),
        cmocka_unit_test(test_parqueue_sunflowers_right_shift),

        cmocka_unit_test(test_parqueue_ninth_wave_id),
        cmocka_unit_test(test_parqueue_ninth_wave_blur),
        cmocka_unit_test(test_parqueue_ninth_wave_Blur),
        cmocka_unit_test(test_parqueue_ninth_wave_BLUR),
        cmocka_unit_test(test_parqueue_ninth_wave_edge),
        cmocka_unit_test(test_parqueue_ninth_wave_sharp),
        cmocka_unit_test(test_parqueue_ninth_wave_negative),
        cmocka_unit_test(test_parqueue_ninth_wave_emboss),
        cmocka_unit_test(test_parqueue_ninth_wave_left_shift),
        cmocka_unit_test(test_parqueue_ninth_wave_right_shift),
    };

    int result = cmocka_run_group_tests(tests, NULL, NULL);
    lcl_free_filters();
    return result;
}