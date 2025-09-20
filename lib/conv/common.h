#include <pthread.h>
#include "../libbmp/libbmp.h"

#ifndef LCL_COMMON_H
#define LCL_COMMON_H

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
#define absl(a) (a > 0 ? a : -a)

#define MAX_THREADS 1024

/**
 * Return codes of lcl library.
 * OK code is 0.
 * Codes between -1 and -4 are reserved for libbmp library.
 * Lcl error codes start from -5.
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

/**
 * A structure that specifies filter matrix.
 *
 * `data` - a column-major matrix of size `height` * `width`.
 * `factor`, `bias` - after applying the filter, the factor will be
 * multiplied with the result, and the bias added to it. Default filters
 * have `factor` set to 1.0 and `bias` set to 0.0.
 * `width` - number of elements in x-dimension.
 * `height` - number of elements in y-dimension .
 */
typedef struct
{
    double** data;
    double factor;
    double bias;
    unsigned int width;
    unsigned int height;
} lcl_filter_t;

/**
 * A structure that specifies the range of pixels of source image
 * for a single thread to work on. Is used only when
 * `enum lcl_conv_mode mode` == `pile`.
 * 
 * [`start_w`, `end_w`) - pixels in x dimension.
 * [`start_h`, `end_h`) - pixels in y dimension.
 */
typedef struct {
    unsigned int start_w;
    unsigned int start_h;
    unsigned int end_w;
    unsigned int end_h;
} lcl_pile_t;

/** A number that specifies how to divide work between threads.
 * 
 * `pixel` - every thread with `thread_id` = `i` works on every `i`'th pixel,
 * from left to right, up to bottom.
 * `row` - every thread with `thread_id` = `i` works on every `i`'th row,
 * from up to bottom.
 * `column` - every thread with `thread_id` = `i` works on every `i`'th column,
 * from left to right.
 * `pile` - thread works on a rectangle specified by `lcl_pile_t pile`.
 */
enum lcl_conv_mode {
    pixelwise,
    rowwise,
    columnwise,
    pilewise,
};

enum lcl_thread_kind {
    reader,
    writer,
    worker,
    multiple,
};

/**
 * A structure specifying number of threads of each kind to execute.
 * 
 * `readers_num` - number of concurrent readers, 1 reader per image.
 * `foremen_num` - number of concurrently convoluted images.
 * `workers_num` - number of threads convoluting one image.
 * `writers_num` - number of concurrent writers, 1 writer per image.
 */
typedef struct thread_jobs {
    int readers_num;
    int foremen_num;
    int workers_num;
    int writers_num;
} thread_jobs_t;


/**
 * A structure that specifies information needed for a thread
 * to perform an operation on an image.
 * 
 * `pile` - specifies a rectangle for a thread to work on. Is used only when
 * `mode` == `pile`.
 * `filter` - a pointer that specifies filter to be applied.
 * `src` - a pointer that specifies source bmp image.
 * Should be allocated with `bmp_img_read`.
 * `targ` - a pointer that specifies target bmp image.
 * Should be allocated with `bmp_img_read`.
 * `mode` - a number that specifies how to divide work between threads.
 * `thread_kind` - a number that specifies the kind of thread.
 * `thread_id` - a unique number to identify this thread. Should be in range
 * [0, `total_threads`).
 * `total_threads` - total number of threads that are simultaneously executing
 * some function.
 */
struct lcl_arg {
    lcl_pile_t pile;
    const lcl_filter_t* filter;
    const bmp_img* src;
    bmp_img* targ;
    enum lcl_conv_mode mode;
    enum lcl_thread_kind thread_kind;
    unsigned int thread_id;
    unsigned int total_threads;
};

/*
 * Id filter 3x3.
 */
extern lcl_filter_t id_filter;

/*
 * Blur filter 3x3.
 */
extern lcl_filter_t blur_filter;

/*
 * Blur filter 5x5.
 */
extern lcl_filter_t Blur_filter;

/*
 * Blur filter 7x7.
 */
extern lcl_filter_t BLUR_filter;

/*
 * Edge detection filter 3x3.
 */
extern lcl_filter_t edge_filter;

/*
 * Sharpen filter 3x3.
 */
extern lcl_filter_t sharp_filter;

/*
 * Negative filter 3x3.
 */
extern lcl_filter_t negative_filter;

/*
 * Emboss filter 3x3.
 */
extern lcl_filter_t emboss_filter;


/**
 * Allocate memory for filters and fill them with values.
 * Should be used before working with the filters, the filters should then be
 * freed with `lcl_free_filters`.
 *
 * Returns lcl_return_code.
*/
int lcl_init_filters(void);

/**
 * Free the memory taken by filters.
 * Should be used after `lcl_init_filters` when filters are no longer needed.
 */
void lcl_free_filters(void);

/**
 * Apply filter to image, sequentially.
 * 
 * `filter` - a pointer that specifies filter to be applied.
 * `src` - a pointer that specifies source bmp image.
 * Should already be allocated with `bmp_img_read`.
 * `targ` - a pointer that specifies target bmp image.
 * Should already be allocated with `bmp_img_read`.
 *
 * Returns 0 and fills up `targ` on success.
 * Returns `lcl_return_code` on error.
 */
int lcl_app_filter_seq(const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ);

/**
 * Apply filter to an image, concurrently.
 * 
 * `mode` - a number that specifies how to divide work between threads.
 * `nthreads` - number of threads to execute.
 * `filter` - a pointer that specifies filter to be applied.
 * `src` - `bmp_img` pointer that specifies source bmp image.
 * Should already be allocated with `bmp_img_read`.
 * `targ` - `bmp_img` pointer that specifies target bmp image.
 * Should already be allocated with `bmp_img_read`.
 *
 * Returns 0 and fills up `targ` on success.
 * Returns `lcl_return_code` on error.
 */
int lcl_app_filter(enum lcl_conv_mode mode, unsigned int nthreads,
    const lcl_filter_t* filter, const bmp_img* src, bmp_img* targ);

/**
 * Read images from the filesystem, convolute them and
 * write back to the filesystem.
 * 
 * `src_paths` - a pointer to array of paths in filesystem with source images.
 * `targ_paths` - a pointer to array of paths in filesystem for output images.
 * `modes` - a pointer to array of convolution modes for every according image.
 * `filters` - a pointer to array of filters to apply to every according image.
 * `img_num` - number of images. Every array argument should be of its size.
 * `jobs` - structure specifying number of threads of each kind to execute.
 *
 * Returns 0 and saves images on `targ_paths` on success.
 * Returns `lcl_return_code` on error.
 */
int lcl_conv_array(char** src_paths, char** targ_paths, enum lcl_conv_mode* modes,
                    lcl_filter_t** filters, int img_num, thread_jobs_t jobs);

#endif // LCL_COMMON_H
