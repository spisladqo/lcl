#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../conv/common.h"
#include "parse.h"

int main(int argc, char *argv[]) {
    lcl_parse_arg args;

    int err = lcl_parse(argc, argv, &args);
    if (err) {
        return err;
    }

    bmp_img src, targ;
    bmp_img_init_df(&src, 0, 0);
    bmp_img_init_df(&targ, 0, 0);

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

    if (strcmp(args.mode, "seq") == 0) {
        err = lcl_app_filter_seq(args.filter, &src, &targ);
    } else if (strcmp(args.mode, "par") == 0) {
        err =
            lcl_app_filter(args.conv, args.nthreads, args.filter, &src, &targ);
    } else {
        printf("error: unknown mode %s\n", args.mode);
        return INVALID_KEY;
    }

    if (err) {
        return err;
    }

    err = bmp_img_write(&targ, args.targ);
    if (err) {
        printf("could not write img, error %d\n", err);
    }

    bmp_img_free(&src);
    bmp_img_free(&targ);

    return err;
}