#include "image.h"
#include "gray_image.h"
#include "rgb_image.h"
#include "photo_mosaic.h"
#include "bit_field_filter.h"

int main(int argc, char *argv[]){
    Image *img1 = new GrayImage();
    img1->LoadImage("Image-Folder/mnist/img_100.jpg");
    img1->DumpImage("img1.jpg");
    img1->Display_ASCII();
    img1->Display_CMD();
    

    Image *img2 = new RGBImage();
    img2->LoadImage("Image-Folder/lena.jpg");
    img2->DumpImage("img2.jpg");
    img2->Display_ASCII();
    img2->Display_CMD();

    // ---- Step 3: bit-field filter demo (BLUR) ----
    // Load a fresh RGB image, enable BLUR via bitwise OR, apply through the
    // Image* base pointer (double dispatch), then show the blurred result.
    Image *img3 = new RGBImage();
    img3->LoadImage("Image-Folder/lena.jpg");

    BitFieldFilter filter;
    filter.enable(BitFieldFilter::BLUR);
    img3->ApplyFilters(filter);
    img3->Display_CMD();

    // ---- Step 3: bit-field filter demo (SOBEL) ----
    // Fresh RGB image, enable SOBEL, apply through Image*, show the edges.
    Image *img4 = new RGBImage();
    img4->LoadImage("Image-Folder/lena.jpg");

    BitFieldFilter edge_filter;
    edge_filter.enable(BitFieldFilter::SOBEL);
    img4->ApplyFilters(edge_filter);
    img4->Display_CMD();

    // ---- Step 3: bit-field filter demo (CONTRAST) ----
    // Fresh RGB image, enable CONTRAST, apply through Image*, show the result.
    Image *img5 = new RGBImage();
    img5->LoadImage("Image-Folder/lena.jpg");

    BitFieldFilter contrast_filter;
    contrast_filter.enable(BitFieldFilter::CONTRAST);
    img5->ApplyFilters(contrast_filter);
    img5->Display_CMD();

    // ---- Step 3: bit-field filter demo (MOSAIC) ----
    // Fresh RGB image, enable MOSAIC, apply through Image*, show the pixelation.
    Image *img6 = new RGBImage();
    img6->LoadImage("Image-Folder/lena.jpg");

    BitFieldFilter mosaic_filter;
    mosaic_filter.enable(BitFieldFilter::MOSAIC);
    img6->ApplyFilters(mosaic_filter);
    img6->Display_CMD();

    // ---- Step 3: bit-field filter demo (combined: BLUR | SOBEL) ----
    // One filter selects two options via bitwise OR; RunOn applies them in the
    // fixed order BLUR -> SOBEL (blur first, then detect edges on the blurred image).
    Image *img7 = new RGBImage();
    img7->LoadImage("Image-Folder/lena.jpg");

    BitFieldFilter combo_filter;
    combo_filter.enable(BitFieldFilter::BLUR | BitFieldFilter::SOBEL);
    img7->ApplyFilters(combo_filter);
    img7->Display_CMD();

    // some photo mosaic driven code here

    // more ...

    delete img1;
    delete img2;
    delete img3;
    delete img4;
    delete img5;
    delete img6;
    delete img7;
    return 0;
}
