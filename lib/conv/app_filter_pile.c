#include "../libbmp/libbmp.h"
#include "common.h"
#include <pthread.h>
#include <stdlib.h>

void* _lcl_app_filter_pile(void* varg) {
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

    pthread_mutex_lock(&arg->lock);
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
    pthread_mutex_unlock(&arg->lock);
    *ret = LCL_OK;
    // printf("Thread with id %d finished work!", arg->id);

    return ret;
}

int lcl_app_filter_pile_1(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {
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
        .thread_id = 0,
    };

    pthread_t thread;
    pthread_create(&thread, NULL, _lcl_app_filter_pile, &arg);
    pthread_join(thread, &ret);
    err = *(int*)ret;
    free(ret);

    return err;
}

int lcl_app_filter_pile_2(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {
    const int TNUM = 2;
    void* rets[TNUM];
    struct arg args[TNUM];
    int errs[TNUM];
    pthread_t threads[TNUM];
    pthread_t locks[TNUM];

    unsigned int src_end_w = src->img_header.biWidth;
    unsigned int src_end_h = src->img_header.biHeight;

    for (int i = 0; i < TNUM; i++) {
        lcl_pile_t pile = {
            .start_w = i * (src_end_w / 2),
            .start_h = 0,
            .end_w = (i + 1) * (src_end_w / 2),
            .end_h = src_end_h,
        };
        args[i] = (struct arg) {
            .filter = filter,
            .src = src,
            .targ = targ,
            .pile = pile,
            .thread_id = i,
        };
        pthread_mutex_init(&args[i].lock, NULL);

        pthread_create(&threads[i], NULL, _lcl_app_filter_pile, &args);
    }

    for (int i = 0; i < TNUM; i++) {
        pthread_mutex_destroy(&args[i].lock);
        pthread_join(threads[i], &rets[i]);
        errs[i] = *(int*)rets[i];
        free(rets[i]);
    }

    for (int i = 0; i < TNUM; i++) {
        if (errs[i]) return errs[i];
    }

    return errs[0];
}

// int lcl_app_filter_pile_4(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ) {
//     void *ret1, *ret2, *ret3, *ret4;
//     int err1, err2, err3, err4;
//     unsigned int curr_id = 1;

//     unsigned int src_end_w = src->img_header.biWidth;
//     unsigned int src_end_h = src->img_header.biHeight;

//     lcl_pile_t pile1 = {
//         .start_w = 0,
//         .start_h = 0,
//         .end_w = src_end_w / 2,
//         .end_h = src_end_h / 2,
//     };
//     lcl_pile_t pile2 = {
//         .start_w = src_end_w / 2 + 1,
//         .start_h = 0,
//         .end_w = src_end_w,
//         .end_h = src_end_h / 2,
//     };
//     lcl_pile_t pile3 = {
//         .start_w = 0,
//         .start_h = src_end_h / 2 + 1,
//         .end_w = src_end_w / 2,
//         .end_h = src_end_h,
//     };
//     lcl_pile_t pile4 = {
//         .start_w = src_end_w / 2 + 1,
//         .start_h = src_end_h / 2 + 1,
//         .end_w = src_end_w,
//         .end_h = src_end_h,
//     };

//     struct arg arg1 = {
//         .filter = filter,
//         .src = src,
//         .targ = targ,
//         .pile = pile1,
//         .id = curr_id++,
//     };
//     struct arg arg2 = {
//         .filter = filter,
//         .src = src,
//         .targ = targ,
//         .pile = pile2,
//         .id = curr_id++,
//     };
//     struct arg arg3 = {
//         .filter = filter,
//         .src = src,
//         .targ = targ,
//         .pile = pile3,
//         .id = curr_id++,
//     };
//     struct arg arg4 = {
//         .filter = filter,
//         .src = src,
//         .targ = targ,
//         .pile = pile4,
//         .id = curr_id++,
//     };

//     pthread_t thread1, thread2, thread3, thread4;
//     pthread_create(&thread1, NULL, _lcl_app_filter_pile, &arg1);
//     pthread_create(&thread2, NULL, _lcl_app_filter_pile, &arg2);
//     pthread_create(&thread3, NULL, _lcl_app_filter_pile, &arg3);
//     pthread_create(&thread4, NULL, _lcl_app_filter_pile, &arg4);
//     pthread_join(thread1, &ret1);
//     pthread_join(thread2, &ret2);
//     pthread_join(thread3, &ret3);
//     pthread_join(thread4, &ret4);
//     err1 = *(int*)ret1;
//     err2 = *(int*)ret2;
//     free(ret1);
//     free(ret2);
//     free(ret3);
//     free(ret4);

//     return err1;
// }
