#include "../../lib/conv/common.h"
#include "../../lib/libbmp/libbmp.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define LCL_DIR "/home/spisladqo/projects/c/parallels/lcl/"
#define IN_IMG_DIR LCL_DIR "images/input/"
#define OUT_IMG_DIR LCL_DIR "images/output/serial/"

#define NSEC_IN_SEC 1000000000.0

static int test_app_filter(int (*app_filter) (const lcl_filter_t*, const bmp_img*, bmp_img*, unsigned int),
        unsigned int n, lcl_filter_t* filter, const char* fname_src, const char* fname_targ, double *elapsed_s) {
    struct timespec start, end;
    // double start, end;
    double sec, nsec;
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

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    ret = app_filter(filter, &src, &targ, n);

    if (clock_gettime(CLOCK_REALTIME, &end) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    nsec = (end.tv_nsec - start.tv_nsec);
    sec = (end.tv_sec - start.tv_sec);
    *elapsed_s = sec + nsec / NSEC_IN_SEC;

    ret = bmp_img_write(&targ, fname_targ);
    if (ret) {
        printf("could not write img, error %d\n", ret);
        return ret;
    }

    return LCL_OK;
}

#define FILTER BLUR_filter
#define FILT_PREF "BLUR_"
#define IMG_FILENAME "Mona_Lisa.bmp"

#define IN_IMG_PATH (IN_IMG_DIR IMG_FILENAME)
#define OUT_IMG_PATH (OUT_IMG_DIR FILT_PREF IMG_FILENAME)

int main() {
    lcl_init_filters();
    double seconds;
    int err;

    int count = 5;
    int n = 1;

    printf("Test: pile\n");
    for (int i = 0; i < count; i++) {
        err = test_app_filter(lcl_app_filter_pile_n, n, &FILTER, IN_IMG_PATH, OUT_IMG_PATH, &seconds);
        if (err != LCL_OK) {
            printf("Error %d\n", err);
            lcl_free_filters();
            return err;
        }
        printf("Time elapsed, piles (n=%d): %lf s\n", n, seconds);
        n *= 2;
    }

    // printf("Test: pixel\n");
    // for (int i = 0; i < count; i++) {
    //     err = test_app_filter(lcl_app_filter_pixel_n, n, &FILTER, IN_IMG_PATH, OUT_IMG_PATH, &seconds);
    //     if (err != LCL_OK) {
    //         printf("Error %d\n", err);
    //         lcl_free_filters();
    //         return err;
    //     }
    //     printf("Time elapsed, pixels (n=%d): %lf s\n", n, seconds);
    //     n *= 2;
    // }

    lcl_free_filters();
    return 0;
}
