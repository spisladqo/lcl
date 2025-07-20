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

    // for (int x = pile.start_w; x < w; x++) {
    //     for (int y = pile.start_h; y < h; y++) {
    //         double red = 0.0;
    //         double green = 0.0;
    //         double blue = 0.0;

    //         for (int filter_y = 0; filter_y < filter_h; filter_y++) {
    //             for (int filter_x = 0; filter_x < filter_w; filter_x++) {
    //                 int img_x = (x - filter_w / 2 + filter_x + w) % w;
    //                 int img_y = (y - filter_h / 2 + filter_y + h) % h;

    //                 red += src->img_pixels[img_y][img_x].red * filter->data[filter_y][filter_x];
    //                 green += src->img_pixels[img_y][img_x].green * filter->data[filter_y][filter_x];
    //                 blue += src->img_pixels[img_y][img_x].blue * filter->data[filter_y][filter_x];
    //             }
    //         }
    //         unsigned char new_red = min(absl(filter->factor * red + filter->bias), 255);
    //         unsigned char new_green = min(absl(filter->factor * green + filter->bias), 255);
    //         unsigned char new_blue = min(absl(filter->factor * blue + filter->bias), 255);

    //         targ->img_pixels[y][x].red = new_red;
    //         targ->img_pixels[y][x].green = new_green;
    //         targ->img_pixels[y][x].blue = new_blue;
    //     }
    // }

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

int lcl_app_filter_pixel_1(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {
    void *ret;
    int err;

    struct arg arg = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .pile = pile,
        .thread_id = 1,
        .total_threads = 1
    };

    pthread_t thread;
    pthread_create(&thread, NULL, _lcl_app_filter_pixel, &arg);
    pthread_join(thread, &ret);
    err = *(int*)ret;
    free(ret);

    return err;
}

int lcl_app_filter_pixel_2(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {
    void *ret1, *ret2;
    int err1, err2;

    struct arg arg1 = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .thread_id = 0,
        .total_threads = 2,
    };
    struct arg arg2 = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .thread_id = 1,
        .total_threads = 2,
    };

    pthread_t thread1, thread2;
    pthread_create(&thread2, NULL, _lcl_app_filter_pixel, &arg2);
    pthread_create(&thread1, NULL, _lcl_app_filter_pixel, &arg1);
    pthread_join(thread2, &ret2);
    pthread_join(thread1, &ret1);
    err1 = *(int*)ret1;
    err2 = *(int*)ret2;
    free(ret1);
    free(ret2);

    return err1;
}

int lcl_app_filter_pixel_4(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {
    const size_t THREAD_NUM = 4;
    void *ret1, *ret2, *ret3, *ret4;
    int err1, err2, err3, err4;

    struct arg arg1 = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .thread_id = 0,
        .total_threads = 4,
    };
    struct arg arg2 = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .thread_id = 1,
        .total_threads = 4,
    };
    struct arg arg3 = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .thread_id = 2,
        .total_threads = 4,
    };
    struct arg arg4 = {
        .filter = filter,
        .src = src,
        .targ = targ,
        .thread_id = 3,
        .total_threads = 4,
    };

    pthread_t thread1, thread2, thread3, thread4;

    pthread_create(&thread1, NULL, _lcl_app_filter_pixel, &arg1);
    pthread_create(&thread2, NULL, _lcl_app_filter_pixel, &arg2);
    pthread_create(&thread3, NULL, _lcl_app_filter_pixel, &arg3);
    pthread_create(&thread4, NULL, _lcl_app_filter_pixel, &arg4);
    pthread_join(thread1, &ret1);
    pthread_join(thread2, &ret2);
    pthread_join(thread3, &ret3);
    pthread_join(thread4, &ret4);
    err1 = *(int*)ret1;
    err2 = *(int*)ret2;
    err3 = *(int*)ret3;
    err4 = *(int*)ret4;
    free(ret1);
    free(ret2);
    free(ret3);
    free(ret4);

    return err1;
}
