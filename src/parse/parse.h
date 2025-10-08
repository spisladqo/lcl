#include "../lib/conv/common.h"
#define NOT_ENOUGH_MODES 1
#define INVALID_KEY 2
#define INVALID_VAL 3

typedef struct {
    char *mode;
    char *src;
    char *targ;
    lcl_filter_t *filter;
    enum lcl_conv_mode conv;
    int nthreads;
} lcl_parse_arg;

int lcl_parse(int argc, char *argv[], lcl_parse_arg *args);
