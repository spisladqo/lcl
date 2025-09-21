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
    printf("Required arguments:\n");
    printf("  --mode=<mode>           Processing mode (seq, par)\n");
    printf("  --src=<source>          Source image file path\n");
    printf("  --target=<target>       Target image file path\n");
    printf("  --filter=<filter>       Filter to apply:\n");
    printf(
        "                          id, blur, Blur, BLUR, edge, sharp, "
        "negative, emboss, etc.\n\n");
    printf("Required Arguments if mode is par:\n");
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

int lcl_parse(int argc, char *argv[], lcl_parse_arg *args) {
    int opt;
    int option_index = 0;

    args->mode = NULL;
    args->src = NULL;
    args->targ = NULL;
    args->filter = NULL;
    args->conv = 0;
    args->nthreads = 1;

    struct option long_options[] = {{"mode", required_argument, 0, 'm'},
                                    {"src", required_argument, 0, 's'},
                                    {"targ", required_argument, 0, 't'},
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
                if (strcmp(optarg, "seq") != 0 && strcmp(optarg, "par") != 0) {
                    printf("error: invalid mode '%s'. Must be seq or par\n",
                           optarg);
                    return -1;
                }
                break;

            case 's':
                args->src = optarg;
                break;

            case 't':
                args->targ = optarg;
                break;

            case 'f':
                if (strcmp(optarg, "id") == 0) {
                    args->filter = &id_filter;
                } else if (strcmp(optarg, "blur") == 0) {
                    args->filter = &blur_filter;
                } else if (strcmp(optarg, "Blur") == 0) {
                    args->filter = &Blur_filter;
                } else if (strcmp(optarg, "BLUR") == 0) {
                    args->filter = &BLUR_filter;
                } else if (strcmp(optarg, "edge") == 0) {
                    args->filter = &edge_filter;
                } else if (strcmp(optarg, "sharp") == 0) {
                    args->filter = &sharp_filter;
                } else if (strcmp(optarg, "negative") == 0) {
                    args->filter = &negative_filter;
                } else if (strcmp(optarg, "emboss") == 0) {
                    args->filter = &emboss_filter;
                } else {
                    fprintf(stderr,
                            "error: Unknown filter '%s'. Please check "
                            "lib/conv/common.h to "
                            "find the list of filters to use. For example, "
                            "--filter=id).\n",
                            optarg);
                    return -1;
                }
                break;

            case 'c':
                if (strcmp(optarg, "pixelwise") == 0) {
                    args->conv = pixelwise;
                } else if (strcmp(optarg, "pilewise") == 0) {
                    args->conv = pilewise;
                } else if (strcmp(optarg, "rowwise") == 0) {
                    args->conv = rowwise;
                } else if (strcmp(optarg, "columnwise") == 0) {
                    args->conv = columnwise;
                } else {
                    printf(
                        "error: invalid conv '%s'. Must be one of the "
                        "following: "
                        "pixelwise, pilewise, rowwise, columnwise\n",
                        optarg);
                    return INVALID_VAL;
                }
                break;
            case 'n':
                args->nthreads = atoi(optarg);
                if (args->nthreads <= 0) {
                    printf("error: nthreads must be a positive integer\n");
                    return INVALID_VAL;
                }
                break;
            case '?':
                return INVALID_VAL;
            default:
                return INVALID_VAL;
        }
    }
    return 0;
}