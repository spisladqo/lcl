#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FACT_DF 1.0
#define BIAS_DF 0.0

/*
 * Simple 3x3 id filter
 */
double _id_filter_data_df[3][3] = {
    {0, 0, 0},
    {0, 1, 0},
    {0, 0, 0},
};

double *id_filter_data_df[3] = {
    _id_filter_data_df[0],
    _id_filter_data_df[1],
    _id_filter_data_df[2],
};

lcl_filter_t id_filter;

/*
 * Blur filters: blur (3x3), Blur (5x5), BLUR (7x7)
 */
double _blur_filter_data_df[3][3] = {
    {0, 0.2, 0},
    {0.2, 0.2, 0.2},
    {0, 0.2, 0},
};

double *blur_filter_data_df[3] = {
    _blur_filter_data_df[0],
    _blur_filter_data_df[1],
    _blur_filter_data_df[2],
};

lcl_filter_t blur_filter;

double _Blur_filter_data_df[5][5] = {
    {0, 0, 0.077, 0, 0},
    {0, 0.077, 0.077, 0.077, 0},
    {0.077, 0.077, 0.077, 0.077, 0.077},
    {0, 0.077, 0.077, 0.077, 0},
    {0, 0, 0.077, 0, 0},
};

double *Blur_filter_data_df[5] = {
    _Blur_filter_data_df[0], _Blur_filter_data_df[1], _Blur_filter_data_df[2],
    _Blur_filter_data_df[3], _Blur_filter_data_df[4],
};

lcl_filter_t Blur_filter;

double _BLUR_filter_data_df[7][7] = {
    {0, 0, 0, 0.04, 0, 0, 0},
    {0, 0, 0.04, 0.04, 0.04, 0, 0},
    {0, 0.04, 0.04, 0.04, 0.04, 0.04, 0},
    {0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04},
    {0, 0.04, 0.04, 0.04, 0.04, 0.04, 0},
    {0, 0, 0.04, 0.04, 0.04, 0, 0},
    {0, 0, 0, 0.04, 0, 0, 0},
};

double *BLUR_filter_data_df[7] = {
    _BLUR_filter_data_df[0], _BLUR_filter_data_df[1], _BLUR_filter_data_df[2],
    _BLUR_filter_data_df[3], _BLUR_filter_data_df[4], _BLUR_filter_data_df[5],
    _BLUR_filter_data_df[6],
};

lcl_filter_t BLUR_filter;

/*
 * Edge detection filter
 */
double _edge_filter_data_df[3][3] = {
    {-1, -1, -1},
    {-1, 8, -1},
    {-1, -1, -1},
};

double *edge_filter_data_df[3] = {
    _edge_filter_data_df[0],
    _edge_filter_data_df[1],
    _edge_filter_data_df[2],
};

lcl_filter_t edge_filter;

/*
 * Sharpen filter
 */
double _sharp_filter_data_df[3][3] = {
    {0, -1, 0},
    {-1, 5, -1},
    {0, -1, 0},
};

double *sharp_filter_data_df[3] = {
    _sharp_filter_data_df[0],
    _sharp_filter_data_df[1],
    _sharp_filter_data_df[2],
};

lcl_filter_t sharp_filter;

/*
 * Negative filter
 */
double _negative_filter_data_df[3][3] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
};

double *negative_filter_data_df[3] = {
    _negative_filter_data_df[0],
    _negative_filter_data_df[1],
    _negative_filter_data_df[2],
};

lcl_filter_t negative_filter;

/*
 * Emboss filter
 */
double _emboss_filter_data_df[3][3] = {
    {-1, -1, 0},
    {-1, 0, 1},
    {0, 1, 1},
};

double *emboss_filter_data_df[3] = {
    _emboss_filter_data_df[0],
    _emboss_filter_data_df[1],
    _emboss_filter_data_df[2],
};

lcl_filter_t emboss_filter;

/*
 * Left shift filter
 */
double _left_shift_data_df[3][3] = {
    {0, 1, 0},
    {0, 0, 0},
    {0, 0, 0},
};

double *left_shift_data_df[3] = {
    _left_shift_data_df[0],
    _left_shift_data_df[1],
    _left_shift_data_df[2],
};

lcl_filter_t left_shift_filter;

/*
 * Right shift filter
 */
double _right_shift_data_df[3][3] = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 1, 0},
};

double *right_shift_data_df[3] = {
    _right_shift_data_df[0],
    _right_shift_data_df[1],
    _right_shift_data_df[2],
};

lcl_filter_t right_shift_filter;

static int init_filter(lcl_filter_t *filter, double **data_arr, double factor,
                       double bias, int width, int height) {
    double **data = malloc(sizeof(double *) * width);
    if (!data) {
        return LCL_NOT_ENOUGH_MEMORY;
    }
    for (int i = 0; i < width; i++) {
        data[i] = NULL;
    }

    for (int i = 0; i < width; i++) {
        double *col = malloc(sizeof(double) * height);
        if (!col) {
            for (int j = 0; j < i; j++) {
                free(data[j]);
            }
            free(data);
            return LCL_NOT_ENOUGH_MEMORY;
        }
        memcpy(col, data_arr[i], sizeof(double) * height);

        data[i] = col;
    }

    filter->data = data;
    filter->factor = factor;
    filter->bias = bias;
    filter->width = width;
    filter->height = height;

    return LCL_OK;
}

static int init_filter_df(lcl_filter_t *filter, double **data_arr, int width,
                          int height) {
    return init_filter(filter, data_arr, FACT_DF, BIAS_DF, width, height);
}

int lcl_init_filters(void) {
    init_filter_df(&id_filter, id_filter_data_df, 3, 3);
    init_filter_df(&blur_filter, blur_filter_data_df, 3, 3);
    init_filter_df(&Blur_filter, Blur_filter_data_df, 5, 5);
    init_filter_df(&BLUR_filter, BLUR_filter_data_df, 7, 7);
    init_filter_df(&edge_filter, edge_filter_data_df, 3, 3);
    init_filter_df(&sharp_filter, sharp_filter_data_df, 3, 3);
    init_filter_df(&negative_filter, negative_filter_data_df, 3, 3);
    init_filter_df(&emboss_filter, emboss_filter_data_df, 3, 3);
    init_filter_df(&left_shift_filter, left_shift_data_df, 3, 3);
    init_filter_df(&right_shift_filter, right_shift_data_df, 3, 3);
}

static void free_filter(lcl_filter_t *filter) {
    if (!filter) {
        return;
    }

    for (int i = 0; i < filter->width; i++) {
        free(filter->data[i]);
    }
    free(filter->data);
    filter->data = NULL;
}

void lcl_free_filters(void) {
    free_filter(&id_filter);
    free_filter(&blur_filter);
    free_filter(&Blur_filter);
    free_filter(&BLUR_filter);
    free_filter(&edge_filter);
    free_filter(&sharp_filter);
    free_filter(&negative_filter);
    free_filter(&emboss_filter);
    free_filter(&left_shift_filter);
    free_filter(&right_shift_filter);
}
