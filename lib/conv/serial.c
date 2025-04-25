#include "../libbmp/libbmp.h"
#include "common.h"

int libconv_app_filter(const libconv_filter_t* filter, const bmp_img* src, bmp_img* targ) {
    if (!filter || !src || !targ) {
        return LIBCONV_INVALID_ARGUMENT;
    }
    if (src->img_header.biWidth != targ->img_header.biWidth ||
        src->img_header.biHeight != targ->img_header.biHeight) {
        return LIBCONV_SRC_TARG_DIFF_SIZES;
    }

    int w = src->img_header.biWidth;
    int h = src->img_header.biHeight;

    int filter_w = filter->width;
    int filter_h = filter->height;

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;

            for (int filter_y = 0; filter_y < filter_h; filter_y++) {
                for (int filter_x = 0; filter_x < filter_w; filter_x++) {
                    int img_x = (x - filter_w / 2 + filter_x + w) % w;
                    int img_y = (y - filter_h / 2 + filter_y + h) % h;

                    red += src->img_pixels[img_y][img_x].red * filter->data[filter_y][filter_x];
                    green += src->img_pixels[img_y][img_x].green * filter->data[filter_y][filter_x];
                    blue += src->img_pixels[img_y][img_x].blue * filter->data[filter_y][filter_x];
                }
            }
            unsigned char new_red = min(absl(filter->factor * red + filter->bias), 255);
            unsigned char new_green = min(absl(filter->factor * green + filter->bias), 255);
            unsigned char new_blue = min(absl(filter->factor * blue + filter->bias), 255);

            targ->img_pixels[y][x].red = new_red;
            targ->img_pixels[y][x].green = new_green;
            targ->img_pixels[y][x].blue = new_blue;
        }
    }
    return LIBCONV_OK;
}
