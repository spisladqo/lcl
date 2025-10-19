#include "common.h"
#define TEST_FAIL 1

#define MODES_NUM 4

char* fname_src = "images/input/Mona_Lisa.bmp";
char* fname_targ = "images/output/Mona_Lisa.bmp";

int test_seq_id_should_not_change_img(void) {
    bmp_img src, targ;
    int ret;
    ret = bmp_img_read(&src, fname_src);
    ret = bmp_img_read(&targ, fname_src);
    ret = lcl_app_filter_seq(&id_filter, &src, &targ);

    ret = bmp_img_write(&targ, fname_targ);
    ret = compare_imgs(&src, &targ);
    if (ret) return TEST_FAIL;

    bmp_img_free(&src);
    bmp_img_free(&targ);

    return LCL_OK;
}

int test_par_id_should_not_change_img(void) {
    bmp_img src, targ;
    enum lcl_conv_mode modes[] = {rowwise, columnwise, pixelwise, pilewise};
    int ret;
    ret = bmp_img_read(&src, fname_src);
    ret = bmp_img_read(&targ, fname_src);

    for (int i = 0; i < MODES_NUM; i++) {
        ret = lcl_app_filter(modes[i], 4, &id_filter, &src, &targ);
        ret = bmp_img_write(&targ, fname_targ);
        ret = compare_imgs(&src, &targ);
        bmp_img_free(&targ);
        bmp_img_free(&src);
        if (ret) return TEST_FAIL;
    }

    return LCL_OK;
}

int test_parq_id_should_not_change_img(void) {
    int ret;

    char* src_paths[] = {fname_src};
    char* targ_paths[] = {fname_targ};

    enum lcl_conv_mode *modes[] = { {rowwise}, {columnwise}, {pixelwise}, {pilewise} };
    lcl_filter_t filters[] = { &id_filter };
    int img_num = 1;
    thread_jobs_t jobs = {1, 1, 1, 1};

    for (int i = 0; i < MODES_NUM; i++) {
        ret = lcl_conv_array(src_paths, targ_paths, modes[i], filters, img_num, jobs);
        if (ret) return TEST_FAIL;
    }

    return LCL_OK;
}
