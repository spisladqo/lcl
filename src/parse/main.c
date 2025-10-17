#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../lib/conv/common.h"
#include "parse.h"

#define NSEC_IN_SEC 1000000000.0
#define MS_IN_SEC 1000.0

int main(int argc, char *argv[]) {
    lcl_parse_arg args;
    char *src_list[MAX_IMG_NUM];
    char *targ_list[MAX_IMG_NUM];
    lcl_filter_t *filter_list[MAX_IMG_NUM];
    enum lcl_conv_mode conv_mode_list[MAX_IMG_NUM];

    struct timespec start, end;
    double sec, nsec;
    bmp_img src, targ;
    int ret;

    lcl_init_filters();


    int err = lcl_parse(argc, argv, &args, src_list, targ_list, filter_list, conv_mode_list);
    if (err) {
        return err;
    }

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        printf("clock gettime error");
        return CLOCK_ERROR;
    }

    if (strcmp(args.mode, "seq") == 0) {
        err = bmp_img_read(&src, args.src);
        if (err) {
            printf("could not read img to src: error %d\n", err);
            return err;
        }
        err = bmp_img_read(&targ, args.src);
        if (err) {
            printf("could not read img to targ: error %d\n", err);
            return err;
        }

        err = lcl_app_filter_seq(args.filter, &src, &targ);

        err = bmp_img_write(&targ, args.targ);
        if (err) {
            printf("could not write img, error %d\n", err);
        }
    } else if (strcmp(args.mode, "par") == 0) {
        err = bmp_img_read(&src, args.src);
        if (err) {
            printf("could not read img to src: error %d\n", err);
            return err;
        }
        err = bmp_img_read(&targ, args.src);
        if (err) {
            printf("could not read img to targ: error %d\n", err);
            return err;
        }

        err = lcl_app_filter(args.conv, args.nthreads, args.filter, &src, &targ);

        err = bmp_img_write(&targ, args.targ);
        if (err) {
            printf("could not write img, error %d\n", err);
        }
    } else if (strcmp(args.mode, "parq") == 0) {
        err = lcl_conv_array(src_list, targ_list, conv_mode_list, filter_list, args.list_size, args.jobs);
    } else {
        printf("error: unknown mode %s\n", args.mode);
        return INVALID_KEY;
    }

    if (clock_gettime(CLOCK_REALTIME, &end) == -1) {
        printf("clock gettime error");
        return CLOCK_ERROR;
    }

    if (err) {
        return err;
    }

    nsec = (end.tv_nsec - start.tv_nsec);
    sec = (end.tv_sec - start.tv_sec);
    double elapsed = sec + nsec / NSEC_IN_SEC;
    elapsed *= MS_IN_SEC;


    printf("%f", elapsed);

    bmp_img_free(&src);
    bmp_img_free(&targ);
    lcl_free_filters();

    return err;
}