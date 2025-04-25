#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define FACT_DF (1.0)
#define BIAS_DF (0.0)

/*
 * Simple 3x3 id filter
 */
double _id_filter_data_df[3][3] = {
    {0, 0, 0},
    {0, 1, 0},
    {0, 0, 0},
};

double* id_filter_data_df[3] = {
    _id_filter_data_df[0],
    _id_filter_data_df[1],
    _id_filter_data_df[2],
};

libconv_filter_t id_filter;

/*
 * Blur filters: blur_ (3x3), Blur_ (5x5), BLUR_ (7x7)
 */
double _blur_filter_data_df[3][3] = {
    {0,   0.2, 0  },
    {0.2, 0.2, 0.2},
    {0,   0.2, 0  },
};

double* blur_filter_data_df[3] = {
    _blur_filter_data_df[0],
    _blur_filter_data_df[1],
    _blur_filter_data_df[2],
};

libconv_filter_t blur_filter;

double _Blur_filter_data_df[5][5] = {
    {0,     0,     0.077, 0,     0    },
    {0,     0.077, 0.077, 0.077, 0    },
    {0.077, 0.077, 0.077, 0.077, 0.077},
    {0,     0.077, 0.077, 0.077, 0    },
    {0,     0,     0.077, 0,     0    },
};

double* Blur_filter_data_df[5] = {
    _Blur_filter_data_df[0],
    _Blur_filter_data_df[1],
    _Blur_filter_data_df[2],
    _Blur_filter_data_df[3],
    _Blur_filter_data_df[4],
};

libconv_filter_t Blur_filter;

double _BLUR_filter_data_df[7][7] = {
    {0,    0,    0,    0.04, 0,    0,    0   },
    {0,    0,    0.04, 0.04, 0.04, 0,    0   },
    {0,    0.04, 0.04, 0.04, 0.04, 0.04, 0   },
    {0.04, 0.04, 0.04, 0.04, 0.04, 0.04, 0.04},
    {0,    0.04, 0.04, 0.04, 0.04, 0.04, 0   },
    {0,    0,    0.04, 0.04, 0.04, 0,    0   },
    {0,    0,    0,    0.04, 0,    0,    0   },
};

double* BLUR_filter_data_df[7] = {
    _BLUR_filter_data_df[0],
    _BLUR_filter_data_df[1],
    _BLUR_filter_data_df[2],
    _BLUR_filter_data_df[3],
    _BLUR_filter_data_df[4],
    _BLUR_filter_data_df[5],
    _BLUR_filter_data_df[6],
};

libconv_filter_t BLUR_filter;

static int init_filter(libconv_filter_t* filter, double** data_arr,
    double factor, double bias, int width, int height) {
    double** data = malloc(sizeof(double*) * width);
    if (!data) {
        return LIBCONV_NOT_ENOUGH_MEMORY;
    }
    for (int i = 0; i < width; i++) {
        data[i] = NULL;
    }

    for (int i = 0; i < width; i++) {
        double* col = malloc(sizeof(double) * height);
        if (!col) {
            for (int j = 0; j < i;j++) {
                free(data[j]);
            }
            free(data);
            return LIBCONV_NOT_ENOUGH_MEMORY;
        }
        memcpy(col, data_arr[i], sizeof(double) * height);

        data[i] = col;
    }

    filter->data = data;
    filter->factor = factor;
    filter->bias = bias;
    filter->width = width;
    filter->height = height;

    return LIBCONV_OK;
}

static int init_filter_df(libconv_filter_t* filter,
    double** data_arr, int width, int height) {
    return init_filter(filter, data_arr, FACT_DF, BIAS_DF, width, height);
}

int libconv_init_filters(void) {
    init_filter_df(&id_filter, id_filter_data_df, 3, 3);
    init_filter_df(&blur_filter, blur_filter_data_df, 3, 3);
    init_filter_df(&Blur_filter, Blur_filter_data_df, 5, 5);
    init_filter_df(&BLUR_filter, BLUR_filter_data_df, 7, 7);
}

static void free_filter(libconv_filter_t* filter) {
    if (!filter) {
        return;
    }

    for (int i = 0; i < filter->width; i++) {
        free(filter->data[i]);
    }
    free(filter->data);
    filter->data = NULL;
}

void libconv_free_filters(void) {
    free_filter(&id_filter);
    free_filter(&blur_filter);
    free_filter(&Blur_filter);
    free_filter(&BLUR_filter);
}
