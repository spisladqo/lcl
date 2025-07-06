#include "../libbmp/libbmp.h"
#include "common.h"
#include <pthread.h>
#include <stdlib.h>

static struct arg {
    const lcl_filter_t* filter;
    const bmp_img* src;
    bmp_img* targ;
    lcl_pile_t pile;
} arg;

static void* lcl_app_filter(void* varg) {
    struct arg* arg = varg;
    const lcl_filter_t* filter = arg->filter;
    const bmp_img* src = arg->src;
    bmp_img* targ = arg->targ;
    lcl_pile_t pile = arg->pile;

    int* ret = malloc(sizeof(int));
    if (!ret) {
        return NULL;
    }

    if (!filter || !src || !targ) {
        *ret = LCL_INVALID_ARGUMENT;
        return ret;
    }
    if (src->img_header.biWidth != targ->img_header.biWidth ||
        src->img_header.biHeight != targ->img_header.biHeight) {
        *ret = LCL_SRC_TARG_DIFF_SIZES;
        return ret;
    }

    int w = pile.end_w;
    int h = pile.end_h;

    int filter_w = filter->width;
    int filter_h = filter->height;

    for (int x = pile.start_w; x < w; x++) {
        for (int y = pile.start_h; y < h; y++) {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;

            for (int filter_y = 0; filter_y < filter_h; filter_y++) {
                for (int filter_x = 0; filter_x < filter_w; filter_x++) {
                    int img_x = (x - filter_w / 2 + filter_x + w) % w;
                    int img_y = (y - filter_h / 2 + filter_y + h) % h;

                    red += src->img_pixels[img_y][img_x].red * filter->data[filter_y][filter_x];
                    green += src->img_pixels[img_y][img_x].green * filter->data[filter_y][filter_x];
                    blue += src->img_pixels[img_y][img_x].blue * filter->data[filter_y][filter_x];
                }
            }
            unsigned char new_red = min(absl(filter->factor * red + filter->bias), 255);
            unsigned char new_green = min(absl(filter->factor * green + filter->bias), 255);
            unsigned char new_blue = min(absl(filter->factor * blue + filter->bias), 255);

            targ->img_pixels[y][x].red = new_red;
            targ->img_pixels[y][x].green = new_green;
            targ->img_pixels[y][x].blue = new_blue;
        }
    }
    *ret = LCL_OK;
    return ret;
}

int lcl_app_filter_one(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {
    void *ret;
    int err;

    lcl_pile_t pile = {
        .start_w = 0,
        .start_h = 0,
        .end_w = src->img_header.biWidth,
        .end_h = src->img_header.biHeight,
    };

    struct arg arg = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .pile = pile,
    };

    pthread_t thread;
    pthread_create(&thread, NULL, lcl_app_filter, &arg);
    pthread_join(thread, &ret);
    err = *(int*)ret;
    free(ret);

    return err;
}