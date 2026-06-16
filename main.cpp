#include "image.h"
#include "gray_image.h"
#include "rgb_image.h"
#include "photo_mosaic.h"
#include "bit_field_filter.h"
#include "image_factory.h"

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

    // ---- Step 5: extra filter demo (BRIGHTNESS) ----
    Image *img8 = new RGBImage();
    img8->LoadImage("Image-Folder/lena.jpg");
    BitFieldFilter bright_filter;
    bright_filter.enable(BitFieldFilter::BRIGHTNESS);
    img8->ApplyFilters(bright_filter);
    img8->Display_CMD();

    // ---- Step 5: extra filter demo (SHARPEN) ----
    Image *img9 = new RGBImage();
    img9->LoadImage("Image-Folder/lena.jpg");
    BitFieldFilter sharpen_filter;
    sharpen_filter.enable(BitFieldFilter::SHARPEN);
    img9->ApplyFilters(sharpen_filter);
    img9->Display_CMD();

    // ---- Step 5: extra filter demo (MEDIAN) ----
    Image *img10 = new RGBImage();
    img10->LoadImage("Image-Folder/lena.jpg");
    BitFieldFilter median_filter;
    median_filter.enable(BitFieldFilter::MEDIAN);
    img10->ApplyFilters(median_filter);
    img10->Display_CMD();

    // ---- Step 5: extra filter demo (INVERT) ----
    Image *img11 = new RGBImage();
    img11->LoadImage("Image-Folder/lena.jpg");
    BitFieldFilter invert_filter;
    invert_filter.enable(BitFieldFilter::INVERT);
    img11->ApplyFilters(invert_filter);
    img11->Display_CMD();

    // ---- Step 5: extra filter demo (GRAYSCALE) ----
    Image *img12 = new RGBImage();
    img12->LoadImage("Image-Folder/lena.jpg");
    BitFieldFilter gray_filter;
    gray_filter.enable(BitFieldFilter::GRAYSCALE);
    img12->ApplyFilters(gray_filter);
    img12->Display_CMD();

    // ---- Step 5: extra filter demo (THRESHOLD) ----
    Image *img13 = new RGBImage();
    img13->LoadImage("Image-Folder/lena.jpg");
    BitFieldFilter thresh_filter;
    thresh_filter.enable(BitFieldFilter::THRESHOLD);
    img13->ApplyFilters(thresh_filter);
    img13->Display_CMD();

    // ---- Step 5: Factory Pattern demo ----
    // ImageFactory::create inspects the file's channels and returns the right
    // concrete type as an Image* (RGB for lena, gray for an mnist digit).
    Image *fac_rgb = ImageFactory::create("Image-Folder/lena.jpg");
    if (fac_rgb) { fac_rgb->Display_CMD(); delete fac_rgb; }
    Image *fac_gray = ImageFactory::create("Image-Folder/mnist/img_100.jpg");
    if (fac_gray) { fac_gray->Display_CMD(); delete fac_gray; }

    // ---- Step 4: PhotoMosaic demo ----
    // Build a mosaic of lena out of the cifar10 tiles; the returned RGBImage* is
    // owned by us, so dump/display then delete it.
    //
    // mosaic_alpha re-blends the original target over the finished mosaic so its
    // outline/contrast re-emerge (the cifar10 tiles skew red, which otherwise
    // washes out the picture). Tweak it here to compare: 0.0 = pure mosaic,
    // ~0.35 = good balance, higher leans toward the original photo.
    const double mosaic_alpha = 0.5;
    const int    mosaic_cell  = 8;
    PhotoMosaic m(mosaic_cell, mosaic_alpha);
    RGBImage *r = m.generate("Image-Folder/cifar10", "Image-Folder/lena.jpg");
    if (r) {
        r->DumpImage("mosaic.jpg");
        r->Display_CMD();
        delete r;
    }

    // ---- Step 5: single-image PhotoMosaic ----
    // Tile source is the set of distinct blocks of ONE image (lena); they rebuild
    // the target (lena) — the IEEE single-image mosaic idea. Returned RGBImage* is
    // owned by us: dump/display then delete.
    PhotoMosaic single_mosaic(8);
    RGBImage *sr = single_mosaic.generate_single("Image-Folder/lena.jpg", "Image-Folder/lena.jpg");
    if (sr) {
        sr->DumpImage("mosaic_single.jpg");
        sr->Display_CMD();
        delete sr;
    }

    // more ...

    delete img1;
    delete img2;
    delete img3;
    delete img4;
    delete img5;
    delete img6;
    delete img7;
    delete img8;
    delete img9;
    delete img10;
    delete img11;
    delete img12;
    delete img13;
    return 0;
}
