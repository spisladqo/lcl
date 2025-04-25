#include <stdlib.h>
#include "../../lib/conv/common.h"
#include "../../lib/libbmp/libbmp.h"

#define LIBCONV_DIR "/home/spisladqo/projects/c/parallels/libconv/"
#define IN_IMG_DIR LIBCONV_DIR "images/input/"
#define OUT_IMG_DIR LIBCONV_DIR "images/output/serial/"

static int test_apply_filter(libconv_filter_t* filter, const char* fname_src, const char* fname_targ) {
    bmp_img src, targ;
    int ret;

    bmp_img_init_df(&src, 0, 0);
    bmp_img_init_df(&targ, 0, 0);

    ret = bmp_img_read(&src, fname_src);
    if (ret) {
        printf("could not read img to src: error %d\n", ret);
        return ret;
    }
    bmp_img_read(&targ, fname_src);
    if (ret) {
        printf("could not read img to targ: error %d\n", ret);
        return ret;
    }

    ret = libconv_app_filter(filter, &src, &targ);
    if (ret) {
        printf("could not apply filter, error %d\n", ret);
        return ret;
    }

    ret = bmp_img_write(&targ, fname_targ);
    if (ret) {
        printf("could not write img, error %d\n", ret);
        return ret;
    }

    return LIBCONV_OK;
}

int main() {
    libconv_init_filters();

    test_apply_filter(&id_filter, IN_IMG_DIR "Mona_Lisa.bmp" , OUT_IMG_DIR "id_Mona_Lisa.bmp" );
    test_apply_filter(&blur_filter, IN_IMG_DIR "Sunflowers_van_Gogh.bmp", OUT_IMG_DIR "blur_Sunflowers_van_Gogh.bmp");
    test_apply_filter(&Blur_filter, IN_IMG_DIR "Impression_Sunrise.bmp", OUT_IMG_DIR "Blur_Impression_Sunrise.bmp");
    test_apply_filter(&BLUR_filter, IN_IMG_DIR "The_Ninth_Wave.bmp", OUT_IMG_DIR "BLUR_The_Ninth_Wave.bmp");
    test_apply_filter(&BLUR_filter, IN_IMG_DIR "Almond_van_Gogh.bmp", OUT_IMG_DIR "BLUR_Almond_van_Gogh.bmp");

    libconv_free_filters();
    return 0;
}
