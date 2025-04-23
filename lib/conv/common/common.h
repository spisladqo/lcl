#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

enum conv_return_code
{
    CONV_OK = 0,
    CONV_INVALID_ARGUMENT,
    CONV_SRC_TARG_DIFF_SIZES = -2,
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
} filter_t;

filter_t id_filter = {
    .factor = 1.0,
    .bias = 0.0,
    .width = 3,
    .height = 3
};

filter_t blur_filter = {
    .factor = 1.0 / 5.0,
    .bias = 0.0,
    .width = 3,
    .height = 3
};
