#include "parse.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_help(const char *name) {
    printf(
        "Usage: %s --mode=<mode> --src=<source> --target=<target> "
        "--filter=<filter> [options]\n\n",
        name);
    printf("Format:\n");
    printf("  --mode=<mode>           Processing mode (seq, par, parq)\n");
    printf("  --src=<source>,<source>,..          Source image file paths\n");
    printf("  --target=<target>,<target>,..       Target image file paths\n");
    printf("  --filter=<filter>       Filter to apply:\n");
    printf(
        "                          id, blur, Blur, BLUR, edge, sharp, "
        "negative, emboss, etc.\n\n");
    printf("Required if mode is par:\n");
    printf(
        "  --conv=<type>           Convolution type (pixelwise, pilewise, "
        "rowwise, columnwise)\n");
    printf("  --nthreads=<n>          Number of threads (default: 1)\n");
        printf("Required if mode is parq:\n");
    printf(
        "  --conv=<type>           Convolution type (pixelwise, pilewise, "
        "rowwise, columnwise)\n");
    printf("  --nthreads=<n>          Number of threads (default: 1)\n");
    
    printf("Optional arguments:\n");
    printf("  --help                  Show this help message\n\n");
    printf("Examples:\n");
    printf(
        "  %s --mode=seq --src=input.bmp --target=output.bmp --filter=blur\n",
        name);
    printf(
        "  %s --mode=par --src=in.bmp --target=out.bmp --filter=edge "
        "--nthreads=4 --conv=pixelwise\n",
        name);
}

int lcl_parse(int argc, char *argv[], lcl_parse_arg *args, char *src_list[MAX_IMG_NUM], char *targ_list[MAX_IMG_NUM],
                lcl_filter_t *filter_list[MAX_IMG_NUM], enum lcl_conv_mode conv_mode_list[MAX_IMG_NUM]) {
    int opt;
    int option_index = 0;

    args->mode = NULL;
    args->src = NULL;
    args->targ = NULL;
    args->filter = NULL;
    args->conv = 0;
    args->nthreads = 1;

    args->src_list = NULL;
    args->targ_list = NULL;
    args->filter_list = NULL;
    args->conv_mode_list = NULL;
    args->list_size = 0;

    int src_num = 0;
    int targ_num = 0;
    int filter_num = 0;
    int conv_num = 0;

    int readers_num, foremen_num, workers_num, writers_num;

    struct option long_options[] = {{"mode", required_argument, 0, 'm'},
                                    {"src", required_argument, 0, 's'},
                                    {"target", required_argument, 0, 't'},
                                    {"filter", required_argument, 0, 'f'},
                                    {"conv", required_argument, 0, 'c'},
                                    {"nthreads", required_argument, 0, 'n'},
                                    {"help", no_argument, 0, 'h'},
                                    {0, 0, 0, 0}};

    if (argc == 1) {
        print_help(argv[0]);
        return 0;
    }

    while ((opt = getopt_long(argc, argv, "m:s:t:f:c:n:", long_options,
                              &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                return 0;
            case 'm':
                args->mode = optarg;
                // printf("read mode %s\n", optarg);
                if (strcmp(optarg, "seq") != 0 && strcmp(optarg, "par") != 0 &&
                    strcmp(optarg, "parq") != 0) {
                    printf("error: invalid mode '%s'. Must be seq, par or parq\n",
                           optarg);
                    return -1;
                }
                break;

            case 's':
                // args->src = optarg;
                // if (strcmp(args->mode, "parq") == 0) {
                char *src = strtok(optarg, ",");
                while (src != NULL) {
                    // printf("read src: %s\n", src);
                    src_list[src_num++] = src;
                    src = strtok(NULL, ",");
                }
                // }
                break;

            case 't':
                // args->targ = optarg;
                // if (strcmp(args->mode, "parq") == 0) {
                char *targ = strtok(optarg, ",");
                while (targ != NULL) {
                    // printf("read targ: %s\n", targ);
                    targ_list[targ_num++] = targ;
                    targ = strtok(NULL, ",");
                }
                // }
                break;

            case 'f':
                    // args->filter = optarg;
                // if (strcmp(args->mode, "parq") == 0) {
                char *filter_str = strtok(optarg, ",");
                while (filter_str != NULL) {
                    // printf("read filter: %s\n", filter_str);
                    if (strcmp(filter_str, "id") == 0) {
                        filter_list[filter_num++] = &id_filter;
                    } else if (strcmp(filter_str, "blur") == 0) {
                        filter_list[filter_num++] = &blur_filter;
                    } else if (strcmp(filter_str, "Blur") == 0) {
                        filter_list[filter_num++] = &Blur_filter;
                    } else if (strcmp(filter_str, "BLUR") == 0) {
                        filter_list[filter_num++] = &BLUR_filter;
                    } else if (strcmp(filter_str, "edge") == 0) {
                        filter_list[filter_num++] = &edge_filter;
                    } else if (strcmp(filter_str, "sharp") == 0) {
                        filter_list[filter_num++] = &sharp_filter;
                    } else if (strcmp(filter_str, "negative") == 0) {
                        filter_list[filter_num++] = &negative_filter;
                    } else if (strcmp(filter_str, "emboss") == 0) {
                        filter_list[filter_num++] = &emboss_filter;
                    } else {
                        printf(
                                "unknown filter '%s'. Please check "
                                "lib/conv/common.h to "
                                "find the list of filters to use. For example, "
                                "--filter=id).\n",
                                filter_str);
                        return -1;
                    }
                    filter_str = strtok(NULL, ",");
                }
                // }
                break;

            case 'c':
                // args->conv = optarg;
                // if (strcmp(args->mode, "parq") == 0) {
                char *conv_str = strtok(optarg, ",");
                while (conv_str != NULL) {
                    // printf("read conv: %s\n", conv_str);
                    if (strcmp(conv_str, "pixelwise") == 0) {
                        conv_mode_list[conv_num++] = pixelwise;
                    } else if (strcmp(conv_str, "pilewise") == 0) {
                        conv_mode_list[conv_num++] = pilewise;
                    } else if (strcmp(conv_str, "rowwise") == 0) {
                        conv_mode_list[conv_num++] = rowwise;
                    } else if (strcmp(conv_str, "columnwise") == 0) {
                        conv_mode_list[conv_num++] = columnwise;
                    } else {
                        printf(
                            "error: invalid conv '%s'. Must be one of the "
                            "following: "
                            "pixelwise, pilewise, rowwise, columnwise\n",
                            conv_str);
                        return INVALID_VAL;
                    }
                    conv_str = strtok(NULL, ",");
                }
                break;
            case 'n':
                if (strcmp(args->mode, "parq") == 0) {
                    char *str = strtok(optarg, ",");
                    int i = 0;
                    while (str != NULL) {
                        if (i == 0) {
                            // printf("readers num: %s\n", str);
                            readers_num = atoi(str);
                        } else if (i == 1) {
                            // printf("foremen num: %s\n", str);
                            foremen_num = atoi(str);
                        } else if (i == 2) {
                            // printf("workers num: %s\n", str);
                            workers_num = atoi(str);
                        } else if (i == 3) {
                            // printf("writers num: %s\n", str);
                            writers_num = atoi(str);
                        } else {
                            printf("error: too many nthread parameters, expected 4\n");
                            return INVALID_VAL;
                        }
                        str = strtok(NULL, ",");
                        i++;
                    }
                    if (readers_num <= 0 || foremen_num <= 0 || workers_num <= 0 || writers_num <= 0) {
                        printf("error: all of thread nums should be positive, but found: %d, %d, %d, %d\n",
                            readers_num, foremen_num, workers_num, writers_num);
                        return INVALID_VAL;
                    }
                    args->jobs = (thread_jobs_t) {
                        .readers_num = readers_num,
                        .foremen_num = foremen_num,
                        .workers_num = workers_num,
                        .writers_num = writers_num
                    };
                } else {
                    args->nthreads = atoi(optarg);
                    if (args->nthreads <= 0) {
                        printf("error: nthreads must be a positive integer\n");
                        return INVALID_VAL;
                    }
                }
                break;
            case '?':
                return INVALID_VAL;
            default:
                return INVALID_VAL;
        }
    }

    if (src_num == targ_num && targ_num == filter_num && filter_num == conv_num &&
        (strcmp(args->mode, "parq") == 0 || strcmp(args->mode, "par") == 0)) {
        args->src_list = src_list;
        args->targ_list = targ_list;
        args->filter_list = filter_list;
        args->conv_mode_list = conv_mode_list;
        args->list_size = src_num;

        args->src = src_list[0];
        args->targ = targ_list[0];
        args->filter = filter_list[0];
        args->conv = conv_mode_list[0];
    } else if (src_num == 1 && src_num == targ_num && targ_num == filter_num && filter_num == conv_num &&
        strcmp(args->mode, "seq") == 0) {
        args->src_list = src_list;
        args->targ_list = targ_list;
        args->filter_list = filter_list;
        args->conv_mode_list = conv_mode_list;
        args->list_size = src_num;
    } else if (strcmp(args->mode, "parq") == 0) {
        printf("error: parq mode requires equal number of all list parameters\n");
        return INVALID_VAL;
    } else if (strcmp(args->mode, "par") == 0 || strcmp(args->mode, "seq") == 0) {
        printf("error: seq par modes require one parameter for each argument parameters, but found:"
            "src: %d, targ: %d, filter: %d, conv: %d\n", src_num, targ_num, filter_num, conv_num);
        return INVALID_VAL;
    }

    return 0;
}