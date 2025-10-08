#include <pthread.h>
#include <stdlib.h>

#include "../libbmp/libbmp.h"
#include "common.h"

static inline void *convolute_pixel(struct lcl_arg *arg, int x, int y,
                                    int img_w, int img_h) {
    bmp_img *src = arg->src;
    bmp_img *targ = arg->targ;
    lcl_filter_t *filter = arg->filter;
    int filter_w = filter->width;
    int filter_h = filter->height;

    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;

    for (int filter_y = 0; filter_y < filter_h; filter_y++) {
        for (int filter_x = 0; filter_x < filter_w; filter_x++) {
            int img_x = (x - filter_w / 2 + filter_x + img_w) % img_w;
            int img_y = (y - filter_h / 2 + filter_y + img_h) % img_h;

            red += src->img_pixels[img_y][img_x].red *
                   filter->data[filter_y][filter_x];
            green += src->img_pixels[img_y][img_x].green *
                     filter->data[filter_y][filter_x];
            blue += src->img_pixels[img_y][img_x].blue *
                    filter->data[filter_y][filter_x];
        }
    }
    unsigned char new_red = min(absl(filter->factor * red + filter->bias), 255);
    unsigned char new_green =
        min(absl(filter->factor * green + filter->bias), 255);
    unsigned char new_blue =
        min(absl(filter->factor * blue + filter->bias), 255);

    targ->img_pixels[y][x].red = new_red;
    targ->img_pixels[y][x].green = new_green;
    targ->img_pixels[y][x].blue = new_blue;
}

static void *app_filter(void *varg) {
    struct lcl_arg *arg = varg;

    lcl_pile_t pile = arg->pile;
    lcl_filter_t *filter = arg->filter;

    bmp_img *src = arg->src;
    bmp_img *targ = arg->targ;

    enum lcl_conv_mode mode = arg->mode;
    enum lcl_thread_kind thread_kind = arg->thread_kind;

    int thread_id = arg->thread_id;
    int total_threads = arg->total_threads;

    int *ret = malloc(sizeof(int));
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

    int end_w, end_h;
    int w, h;
    int x, y;

    w = targ->img_header.biWidth;
    h = targ->img_header.biHeight;

    if (mode == pixelwise) {
        x = thread_id;
    }

    int filter_w = filter->width;
    int filter_h = filter->height;

    switch (mode) {
        case pilewise:
            for (x = pile.start_w; x < pile.end_w; x++) {
                for (y = pile.start_h; y < pile.end_h; y++) {
                    convolute_pixel(arg, x, y, w, h);
                }
            }
            break;
        case pixelwise:
            for (y = 0; y < h; y++) {
                for (x = thread_id; x < w; x += total_threads) {
                    convolute_pixel(arg, x, y, w, h);
                }
            }
            break;
        case columnwise:
            for (x = thread_id; x < w; x += total_threads) {
                for (y = 0; y < h; y++) {
                    convolute_pixel(arg, x, y, w, h);
                }
            }
        case rowwise:
        default:
            for (y = thread_id; y < h; y += total_threads) {
                for (x = 0; x < w; x++) {
                    convolute_pixel(arg, x, y, w, h);
                }
            }
    }

    *ret = LCL_OK;

    return ret;
}

int lcl_app_filter(enum lcl_conv_mode mode, unsigned int nthreads,
                   const lcl_filter_t *filter, const bmp_img *src,
                   bmp_img *targ) {
    if (nthreads > MAX_THREADS) {
        printf("%u exceeds max number of threads: %u\n", nthreads, MAX_THREADS);
        return LCL_INVALID_ARGUMENT;
    }
    if (nthreads < 0) {
        printf("nthreads cannot be less than zero!\n");
        return LCL_INVALID_ARGUMENT;
    }
    if (!filter || !src || !targ) {
        printf("pointer arguments should not be null!\n");
        return LCL_INVALID_ARGUMENT;
    }

    void *rets[nthreads];
    struct lcl_arg args[nthreads];
    int errs[nthreads];
    pthread_t threads[nthreads];

    unsigned int src_end_w = src->img_header.biWidth;
    unsigned int src_end_h = src->img_header.biHeight;

    for (int i = 0; i < nthreads; i++) {
        lcl_pile_t pile = {
            .start_w = 0,
            .end_w = src_end_w,
            .start_h = i * ((src_end_h + nthreads) / nthreads),
            .end_h = (i + 1) * ((src_end_h + nthreads) / nthreads),
        };

        if (pile.end_w > src_end_w) {
            pile.end_w = src_end_w;
        }

        if (pile.end_h > src_end_h) {
            pile.end_h = src_end_h;
        }
        // printf("%d'th thread works on x from %u to %u, on y from %u to %u\n",
        // i, pile.start_w, pile.end_w, pile.start_h, pile.end_h);

        // printf("thread %d works from %d to %d\n", i, pile.start_w,
        // pile.end_w);
        args[i] = (struct lcl_arg){
            .pile = pile,
            .filter = filter,
            .src = src,
            .targ = targ,
            .mode = mode,
            .thread_id = i,
            .total_threads = nthreads,
            .thread_kind = worker,
        };

        pthread_create(&threads[i], NULL, app_filter, &args[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], &rets[i]);
        errs[i] = *(int *)rets[i];
        free(rets[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        if (errs[i]) return errs[i];
    }

    return errs[0];
}
