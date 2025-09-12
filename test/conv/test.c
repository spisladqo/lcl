#include "../../lib/conv/common.h"
#include "../../lib/libbmp/libbmp.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define LCL_DIR "/home/spisladqo/projects/c/parallels/lcl/"
#define IN_IMG_DIR LCL_DIR "images/input/"
#define OUT_IMG_DIR LCL_DIR "images/output/serial/"

#define NSEC_IN_SEC 1000000000.0

static int test_app_filter(enum lcl_work_mode mode, unsigned int n, lcl_filter_t* filter,
    const char* fname_src, const char* fname_targ, double* elapsed_s) {
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

    ret = lcl_app_filter(mode, n, filter, &src, &targ);

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

#define GET_MODE_NAME(mode, name)   \
    switch (mode) {                 \
    case pilewise:                  \
            name = "pilewise";      \
    break;                          \
    case pixelwise:                 \
            name = "pixelwise";     \
    break;                          \
    case rowwise:                   \
            name = "rowwise";       \
    break;                          \
    default:                        \
            name = "columnwise";    \
    }

#define FILTER BLUR_filter
#define FILT_PREF "BLUR_"
#define IMG_FILENAME "Mona_Lisa.bmp"

#define IN_IMG_PATH (IN_IMG_DIR IMG_FILENAME)
#define OUT_IMG_PATH (OUT_IMG_DIR FILT_PREF IMG_FILENAME)
#define MODES_NUM 4

int main() {
    lcl_init_filters();
    double seconds;
    int err;

    int count = 5;
    int n = 1;
    char* name;

    enum lcl_work_mode work_modes[MODES_NUM] = { pilewise, pixelwise, rowwise, columnwise };

    for (int j = 0; j < MODES_NUM; j++) {
        GET_MODE_NAME(work_modes[j], name);
        printf("Test: %s\n", name);

        for (int i = 0; i < count; i++) {
            err = test_app_filter(work_modes[j], n, &FILTER, IN_IMG_PATH, OUT_IMG_PATH, &seconds);
            if (err != LCL_OK) {
                printf("Error %d\n", err);
                lcl_free_filters();
                return err;
            }
            printf("Time elapsed, %s (n=%d): %lf s\n", name, n, seconds);
            n *= 2;
        }
        n = 1;
    }

    lcl_free_filters();
    return 0;
}
