#include "../libbmp/libbmp.h"
#include "common.h"
#include <pthread.h>
#include <stdlib.h>

void* _lcl_app_filter_pixel(void* varg) {
    struct arg* arg = varg;
    const lcl_filter_t* filter = arg->filter;
    const bmp_img* src = arg->src;
    bmp_img* targ = arg->targ;
    unsigned int total_threads = arg->total_threads;

    // printf("Thread with id %d started work!\n", arg->id);

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

    unsigned int w = targ->img_header.biWidth;
    unsigned int h = targ->img_header.biHeight;

    int filter_w = filter->width;
    int filter_h = filter->height;

    size_t x = arg->thread_id;

    for (size_t y = 0; y < h; y++) {
        for (x = x % w; x < w; x += total_threads) {
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

    // printf("Thread with id %d finished work!", arg->id);

    return ret;
}

int lcl_app_filter_pixel_n(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ, unsigned int n) {
    if (n > MAX_THREADS) {
        printf("%u exceeds max number of threads: %u\n", n, MAX_THREADS);
        return LCL_INVALID_ARGUMENT;
    }

    void* rets[n];
    struct arg args[n];
    int errs[n];
    pthread_t threads[n];

    for (int i = 0; i < n; i++) {
        args[i] = (struct arg) {
            .filter = filter,
            .src = src,
            .targ = targ,
            .thread_id = i,
            .total_threads = n
        };

        pthread_create(&threads[i], NULL, _lcl_app_filter_pixel, &args[i]);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], &rets[i]);
        errs[i] = *(int*)rets[i];
        free(rets[i]);
    }

    for (int i = 0; i < n; i++) {
        if (errs[i]) return errs[i];
    }

    return errs[0];
}
