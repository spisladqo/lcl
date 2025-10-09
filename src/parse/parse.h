#include "../lib/conv/common.h"
#define NOT_ENOUGH_MODES 1
#define INVALID_KEY 2
#define INVALID_VAL 3
#define CLOCK_ERROR 4

typedef struct {
    char *mode;
    char *src;
    char *targ;
    lcl_filter_t *filter;
    enum lcl_conv_mode conv;
    int nthreads;

    char **src_list;
    char **targ_list;
    lcl_filter_t **filter_list;
    enum lcl_conv_mode *conv_mode_list;
    thread_jobs_t jobs;
    int list_size;
} lcl_parse_arg;

int lcl_parse(int argc, char *argv[], lcl_parse_arg *args, char** src_list, char** targ_list,
                lcl_filter_t **filter_list, enum lcl_conv_mode *conv_mode_list);