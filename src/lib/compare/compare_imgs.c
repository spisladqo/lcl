#include "compare_imgs.h"

int compare_imgs(const bmp_img *src, const bmp_img *targ) {
    int err = 0;

    for (int x = 0; x < src->img_header.biWidth; x++) {
        for (int y = 0; y < src->img_header.biHeight; y++) {
            bmp_pixel pixel1 = src->img_pixels[y][x];
            bmp_pixel pixel2 = targ->img_pixels[y][x];

            if (pixel1.red != pixel2.red || pixel1.green != pixel2.green ||
                pixel1.blue != pixel2.blue) {
                printf(
                    "pixels on (x:%d, y:%d) don't match:\n src has (r:%d, "
                    "g:%d, b:%d), targ has (r:%d, g:%d, b:%d)",
                    pixel1.red, pixel1.green, pixel1.blue, pixel2.red,
                    pixel2.green, pixel2.blue);

                return COLOR_MISMATCH;
            }
        }
    }
    return 0;
}
