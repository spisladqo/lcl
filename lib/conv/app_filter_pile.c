#include "../libbmp/libbmp.h"
#include "common.h"
#include <pthread.h>
#include <stdlib.h>

static void* _lcl_app_filter_pile(void *varg) {
    struct arg* arg = varg;
    const lcl_filter_t* filter = arg->filter;
    const bmp_img* src = arg->src;
    bmp_img* targ = arg->targ;
    lcl_pile_t pile = arg->pile;

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

    // printf("Thread with id %d finished work!", arg->id);

    return ret;
}

static void* _lcl_app_filter_pixel(void* varg) {
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

static void* _lcl_app_filter_row(void* varg) {
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

    for (size_t y = 0; y < h; y += total_threads) {
        for (size_t x = 0; x < w; x++) {
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

static void* _lcl_app_filter_col(void* varg) {
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

    for (size_t x = 0; x < w; x += total_threads) {
        for (size_t y = 0; y < h; y++) {
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

int lcl_app_filter(enum work_mode mode, unsigned int nthreads,
    const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {

    if (nthreads > MAX_THREADS) {
        printf("%u exceeds max number of threads: %u\n", nthreads, MAX_THREADS);
        return LCL_INVALID_ARGUMENT;
    }

    void* rets[nthreads];
    struct arg args[nthreads];
    int errs[nthreads];
    pthread_t threads[nthreads];

    unsigned int src_end_w = src->img_header.biWidth;
    unsigned int src_end_h = src->img_header.biHeight;

    for (int i = 0; i < nthreads; i++) {
        lcl_pile_t pile = {
            .start_w = i * ((src_end_w + 1) / nthreads),
            .start_h = 0,
            .end_w = (i + 1) * ((src_end_w + 1) / nthreads),
            .end_h = src_end_h,
        };
        // printf("thread %d works from %d to %d\n", i, pile.start_w, pile.end_w);
        args[i] = (struct arg) {
            .pile = pile,
            .filter = filter,
            .src = src,
            .targ = targ,
            .mode = mode,
            .thread_id = i,
            .total_threads = nthreads,
        };

        if (mode == pixel) {
            pthread_create(&threads[i], NULL, _lcl_app_filter_pixel, &args[i]);
        } else if (mode == row) {
            pthread_create(&threads[i], NULL, _lcl_app_filter_row, &args[i]);
        } else if (mode == column) {
            pthread_create(&threads[i], NULL, _lcl_app_filter_col, &args[i]);
        } else {
            pthread_create(&threads[i], NULL, _lcl_app_filter_pile, &args[i]);
        }
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], &rets[i]);
        errs[i] = *(int*)rets[i];
        free(rets[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        if (errs[i]) return errs[i];
    }

    return errs[0];
}
