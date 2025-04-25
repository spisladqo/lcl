#include "../libbmp/libbmp.h"

#ifndef LIBCONV_COMMON_H
#define LIBCONV_COMMON_H

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
#define absl(a) (a > 0 ? a : -a)

/*
 * libbmp uses error codes -1 to -4, so to distinguish their errors,
 * libconv starts from -5
 */
enum libconv_return_code
{
    LIBCONV_OK = 0,
    LIBCONV_INVALID_ARGUMENT = -5,
    LIBCONV_SRC_TARG_DIFF_SIZES,
    LIBCONV_NOT_ENOUGH_MEMORY = -7,
};

/*
 * data should be a COLUMN-major matrix
 */
typedef struct
{
    double** data;
    double factor;
    double bias;
    int width;
    int height;
} libconv_filter_t;

/*
 * 3x3 id filter
 */
extern libconv_filter_t id_filter;

/*
 * 3x3 blur filter
 */
extern libconv_filter_t blur_filter;

/*
 * 5x5 blur filter
 */
extern libconv_filter_t Blur_filter;

/*
 * 7x7 blur filter
 */
extern libconv_filter_t BLUR_filter;

int libconv_init_filters(void);
void libconv_free_filters(void);
int libconv_app_filter(const libconv_filter_t* filter, const bmp_img* src, bmp_img* targ);

#endif // LIBCONV_COMMON_H
