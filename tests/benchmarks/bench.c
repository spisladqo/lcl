#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../lib/conv/common.h"

#define N 20
#define NSEC_IN_SEC 1000000000.0

char *get_test_image_path(const char *filename) {
    static char path[PATH_MAX];
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return NULL;
    }

    const char *paths[] = {"../../images/input/%s", "../images/input/%s",
                           "images/input/%s", "../../../images/input/%s", "%s"};

    for (int i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        snprintf(path, sizeof(path), paths[i], filename);

        if (access(path, F_OK) == 0) {
            return path;
        }
    }

    return NULL;
}

char *get_test_image_out_path(const char *filename) {
    static char path[PATH_MAX];
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return NULL;
    }

    const char *paths[] = {"../../images/output/%s", "../images/output/%s",
                           "images/output/%s", "../../../images/output/%s",
                           "%s"};

    for (int i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        snprintf(path, sizeof(path), paths[i], filename);

        if (access(path, F_OK) == 0) {
            return path;
        }
    }
}

// char* get_output_image_path(const char* filename) {
//     static char path[PATH_MAX];
//     char cwd[PATH_MAX];

//     if (getcwd(cwd, sizeof(cwd)) == NULL) {
//         return NULL;
//     }

//     const char* paths[] = {
//         "../../images/output/%s",
//         "../images/output/%s",
//         "images/output/%s",
//         "../../../images/input/%s",
//         "%s"
//     };

//     for (int i = 0; i < sizeof(paths)/sizeof(paths[0]); i++) {
//         snprintf(path, sizeof(path), paths[i], filename);

//         if (access(path, F_OK) == 0) {
//             return path;
//         }
//     }

//     return NULL;
// }

#define GET_MODE_NAME(mode, name) \
    switch (mode) {               \
        case pilewise:            \
            name = "pilewise";    \
            break;                \
        case pixelwise:           \
            name = "pixelwise";   \
            break;                \
        case rowwise:             \
            name = "rowwise";     \
            break;                \
        case columnwise:          \
            name = "columnwise";  \
            break;                \
        default:                  \
            name = "unknown";     \
    }

typedef struct {
    const char *filter_name;
    lcl_filter_t *filter;
    const char *prefix;
} test_config_t;

static const test_config_t test_configs[] = {
    {"BLUR", &BLUR_filter, "BLUR_"},
    {"EMBOSS", &emboss_filter, "emboss_"},
    {"SHARPEN", &sharp_filter, "sharpen_"},
    {"DETECT EDGES", &edge_filter, "edge_"},
};

#define TEST_CONFIGS_COUNT (sizeof(test_configs) / sizeof(test_configs[0]))

static const char *image_files[] = {"Mona_Lisa.bmp",
                                    // "The_Ninth_Wave.bmp",
                                    // "Almond_van_Gogh.bmp",
                                    "Impression_Sunrise.bmp"};

#define IMAGE_FILES_COUNT (sizeof(image_files) / sizeof(image_files[0]))

static int test_app_filter_seq(lcl_filter_t *filter, const char *fname_src,
                               const char *fname_targ, double *elapsed_s) {
    struct timespec start, end;
    double sec, nsec;
    bmp_img src, targ;
    int ret;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    // bmp_img_init_df(&src, 0, 0);
    // bmp_img_init_df(&targ, 0, 0);

    ret = bmp_img_read(&src, fname_src);
    if (ret) {
        printf("could not read img to src: error %d\n", ret);
        return ret;
    }
    ret = bmp_img_read(&targ, fname_src);
    if (ret) {
        printf("could not read img to targ: error %d\n", ret);
        return ret;
    }

    ret = lcl_app_filter_seq(filter, &src, &targ);

    ret = bmp_img_write(&targ, fname_targ);
    if (ret) {
        printf("could not write img, error %d\n", ret);
    }

    if (clock_gettime(CLOCK_REALTIME, &end) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    nsec = (end.tv_nsec - start.tv_nsec);
    sec = (end.tv_sec - start.tv_sec);
    *elapsed_s = sec + nsec / NSEC_IN_SEC;

    bmp_img_free(&src);
    bmp_img_free(&targ);

    return LCL_OK;
}

static int test_app_filter(enum lcl_conv_mode mode, unsigned int nthreads,
                           lcl_filter_t *filter, const char *fname_src,
                           const char *fname_targ, double *elapsed_s) {
    struct timespec start, end;
    double sec, nsec;
    bmp_img src, targ;
    int ret;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    // bmp_img_init_df(&src, 0, 0);
    // bmp_img_init_df(&targ, 0, 0);

    ret = bmp_img_read(&src, fname_src);
    if (ret) {
        printf("could not read img to src: error %d\n", ret);
        return ret;
    }
    ret = bmp_img_read(&targ, fname_src);
    if (ret) {
        printf("could not read img to targ: error %d\n", ret);
        return ret;
    }

    ret = lcl_app_filter(mode, nthreads, filter, &src, &targ);

    ret = bmp_img_write(&targ, fname_targ);
    if (ret) {
        printf("could not write img, error %d\n", ret);
    }

    if (clock_gettime(CLOCK_REALTIME, &end) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    nsec = (end.tv_nsec - start.tv_nsec);
    sec = (end.tv_sec - start.tv_sec);
    *elapsed_s = sec + nsec / NSEC_IN_SEC;

    bmp_img_free(&src);
    bmp_img_free(&targ);

    return LCL_OK;
}

static int test_conv_array(char **src_paths, char **targ_paths,
                           enum lcl_conv_mode *modes, lcl_filter_t **filters,
                           int img_num, thread_jobs_t jobs, double *elapsed_s) {
    struct timespec start, end;
    double sec, nsec;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    int ret =
        lcl_conv_array(src_paths, targ_paths, modes, filters, img_num, jobs);

    if (clock_gettime(CLOCK_REALTIME, &end) == -1) {
        printf("clock gettime error");
        return LCL_INVALID_ARGUMENT;
    }

    nsec = (end.tv_nsec - start.tv_nsec);
    sec = (end.tv_sec - start.tv_sec);
    *elapsed_s = sec + nsec / NSEC_IN_SEC;

    return ret;
}

#define PATH_IN "/home/spisladqo/projects/c/parallels/lcl/images/input/"
#define PATH_OUT "/home/spisladqo/projects/c/parallels/lcl/images/output/"
#define SMALL_IMG "Almond_van_Gogh.bmp"
#define BIG_IMG "The_Ninth_Wave.bmp"

int main(void) {
    int ret;
    lcl_init_filters();
    double elapsed;
    int thread_num = 4;
    int img_num = 4;

    int mode_num = 3;
    enum lcl_conv_mode modes[] = {pilewise, pixelwise, rowwise, columnwise};
    char *mode_names[] = {"pilewise", "pixelwise", "rowwise", "columnwise"};
    double avg = 0.0;

    printf("small image\n");

    printf("Sequential\n");
    for (int k = 0; k < img_num; k++) {
        for (int i = 0; i < N; i++) {
            // printf("iteration %d, ", i);
            test_app_filter_seq(&blur_filter, PATH_IN SMALL_IMG,
                                PATH_OUT SMALL_IMG, &elapsed);
            printf("%0.4f ", elapsed);
            avg += elapsed;
        }
    }
    avg /= N;
    printf("\navg: %.4f s\n", avg);

    printf("\n\nParallel\n");

    for (int j = 0; j < mode_num; j++) {
        printf("Mode: %s\n", mode_names[j]);

        avg = 0.0;
        for (int k = 0; k < img_num; k++) {
            for (int i = 0; i < N; i++) {
                // printf("iteration %d, ", i);
                test_app_filter(modes[i], thread_num, &blur_filter,
                                PATH_IN SMALL_IMG, PATH_OUT SMALL_IMG,
                                &elapsed);
                printf("%0.4f ", elapsed);
                avg += elapsed;
            }
        }
        avg /= N;
        printf("\navg: %.4f s\n", avg);
    }

    printf("\n\nParallel + Queue\n");
    thread_jobs_t jobs = {.readers_num = img_num,
                          .foremen_num = img_num,
                          .writers_num = img_num,
                          .workers_num = thread_num};

    char *src[] = {PATH_IN SMALL_IMG, PATH_IN "Almond_van_Gogh.bmp",
                   PATH_IN "Impression_Sunrise.bmp",
                   PATH_IN "Sunflowers_van_Gogh.bmp"};
    char *targ[] = {PATH_OUT SMALL_IMG, PATH_OUT "Almond_van_Gogh.bmp",
                    PATH_OUT "Impression_Sunrise.bmp",
                    PATH_OUT "Sunflowers_van_Gogh.bmp"};
    lcl_filter_t *filters[] = {&blur_filter, &blur_filter, &blur_filter,
                               &blur_filter};

    for (int j = 0; j < mode_num; j++) {
        printf("Mode: %s\n", mode_names[j]);
        enum lcl_conv_mode modes_arr[] = {modes[j], modes[j], modes[j],
                                          modes[j]};

        avg = 0.0;
        for (int i = 0; i < N; i++) {
            // printf("iteration %d, ", i);
            test_conv_array(src, targ, modes_arr, filters, img_num, jobs,
                            &elapsed);
            printf("%0.4f ", elapsed);
            avg += elapsed;
        }
        avg /= N;
        printf("\navg: %.4f s\n", avg);
    }

    lcl_free_filters();

    return 0;
}