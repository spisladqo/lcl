#include "common.h"

bmp_img *create_test_image(int width, int height, bmp_pixel pixel) {
    bmp_img *img = malloc(sizeof(bmp_img));
    img->img_header.biWidth = width;
    img->img_header.biHeight = height;

    img->img_pixels = malloc(height * sizeof(bmp_pixel *));
    for (int y = 0; y < height; y++) {
        img->img_pixels[y] = malloc(width * sizeof(bmp_pixel));
        for (int x = 0; x < width; x++) {
            img->img_pixels[y][x] = pixel;
        }
    }
    return img;
}

lcl_filter_t *create_test_filter(int width, int height, double value) {
    lcl_filter_t *filter = malloc(sizeof(lcl_filter_t));
    filter->width = width;
    filter->height = height;
    filter->factor = 1.0;
    filter->bias = 0.0;

    filter->data = malloc(height * sizeof(double *));
    for (int y = 0; y < height; y++) {
        filter->data[y] = malloc(width * sizeof(double));
        for (int x = 0; x < width; x++) {
            filter->data[y][x] = value;
        }
    }
    return filter;
}

void free_test_image(bmp_img *img) {
    if (!img) return;
    for (int y = 0; y < img->img_header.biHeight; y++) {
        free(img->img_pixels[y]);
    }
    free(img->img_pixels);
    free(img);
}

void free_test_filter(lcl_filter_t *filter) {
    if (!filter) return;
    for (int y = 0; y < filter->height; y++) {
        free(filter->data[y]);
    }
    free(filter->data);
    free(filter);
}