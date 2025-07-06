#include "../libbmp/libbmp.h"

#ifndef LCL_COMMON_H
#define LCL_COMMON_H

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
#define absl(a) (a > 0 ? a : -a)

/*
 * libbmp uses error codes -1 to -4, so to distinguish their errors,
 * lcl starts from -5
 */
enum lcl_return_code
{
    LCL_OK = 0,
    LCL_INVALID_ARGUMENT = -5,
    LCL_SRC_TARG_DIFF_SIZES = -6,
    LCL_NOT_ENOUGH_MEMORY,
    LCL_THREAD_CREATE_FAIL,
    LCL_THREAD_JOIN_FAIL,
};

/*
 * data is a column-major matrix
 */
typedef struct
{
    double** data;
    double factor;
    double bias;
    int width;
    int height;
} lcl_filter_t;

typedef struct {
    unsigned int start_w;
    unsigned int start_h;
    unsigned int end_w;
    unsigned int end_h;
} lcl_pile_t;

/*
 * 3x3 id filter
 */
extern lcl_filter_t id_filter;

/*
 * 3x3 blur filter
 */
extern lcl_filter_t blur_filter;

/*
 * 5x5 blur filter
 */
extern lcl_filter_t Blur_filter;

/*
 * 7x7 blur filter
 */
extern lcl_filter_t BLUR_filter;

int lcl_init_filters(void);
void lcl_free_filters(void);
int lcl_app_filter_seq(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ);
int lcl_app_filter_one(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ);

#endif // LCL_COMMON_H
