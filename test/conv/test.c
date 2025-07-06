#include <stdlib.h>
#include "../../lib/conv/common.h"
#include "../../lib/libbmp/libbmp.h"
#include <time.h>

#define LCL_DIR "/home/spisladqo/projects/c/parallels/lcl/"
#define IN_IMG_DIR LCL_DIR "images/input/"
#define OUT_IMG_DIR LCL_DIR "images/output/serial/"

static int test_app_filter(int (*app_filter) (const lcl_filter_t*, const bmp_img*, bmp_img*),
        lcl_filter_t* filter, const char* fname_src, const char* fname_targ, float *elapsed_s) {
    clock_t start, end;
    bmp_img src, targ;
    int ret;

    bmp_img_init_df(&src, 0, 0);
    bmp_img_init_df(&targ, 0, 0);

    ret = bmp_img_read(&src, fname_src);
    if (ret) {
        printf("could not read img to src: error %d\n", ret);
        return ret;
    }
    bmp_img_read(&targ, fname_src);
    if (ret) {
        printf("could not read img to targ: error %d\n", ret);
        return ret;
    }

    do {
        start = clock();
        ret = app_filter(filter, &src, &targ);
        end = clock();
    } while (end < start);

    *elapsed_s = (float)(end - start) / CLOCKS_PER_SEC;
    if (ret) {
        printf("could not apply filter, error %d\n", ret);
        return ret;
    }

    ret = bmp_img_write(&targ, fname_targ);
    if (ret) {
        printf("could not write img, error %d\n", ret);
        return ret;
    }

    return LCL_OK;
}

int main() {
    lcl_init_filters();
    float seconds;
    // seq
    int err = test_app_filter(lcl_app_filter_seq, &id_filter, IN_IMG_DIR "Mona_Lisa.bmp", OUT_IMG_DIR "id_Mona_Lisa.bmp", &seconds);
    if (err != LCL_OK) {
        printf("Error %d\n", err);
        return err;
    }
    printf("Time elapsed, seq: %f s\n", seconds);
    // parallel, 1 thread
     err = test_app_filter(lcl_app_filter_one, &id_filter, IN_IMG_DIR "Mona_Lisa.bmp", OUT_IMG_DIR "id_Mona_Lisa.bmp", &seconds);
    if (err != LCL_OK) {
        printf("Error %d\n", err);
        return err;
    }
    printf("Time elapsed, parallel: %f s\n", seconds);
    lcl_free_filters();

    return 0;
}
