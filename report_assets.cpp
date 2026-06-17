// report_assets.cpp — 產生書面報告所需的「各 Step 結果圖」。
// 這是一支額外的 driver（README 允許自訂其他 driven code），只負責把每個
// Step 的輸出 dump 成圖檔到 report_assets/ 資料夾，方便放進 final_report.pdf。
//
// 用法（在 Linux/Docker 容器內）：
//   make -j                       # 先把 obj/*.o 編好
//   mkdir -p report_assets
//   g++ -I ./inc -I ./third-party/CImg -I ./third-party/libjpeg -I ./Data-Loader \
//       -std=c++11 -O3 report_assets.cpp \
//       obj/bit_field_filter.o obj/data_loader.o obj/gray_image.o obj/image.o \
//       obj/image_factory.o obj/photo_mosaic.o obj/rgb_image.o \
//       -o report_assets -L./third-party/libjpeg -ljpeg -lpng -lm -lpthread
//   ./report_assets > report_assets/ascii_art.txt
//
// 跑完後，report_assets/ 內會有 s2_*.jpg ~ s5_*.jpg 以及 ascii_art.txt。

#include "image.h"
#include "gray_image.h"
#include "rgb_image.h"
#include "photo_mosaic.h"
#include "bit_field_filter.h"
#include "image_factory.h"

#include <cstdlib>   // system
#include <string>

static const std::string OUT = "report_assets/";
static const std::string LENA = "Image-Folder/lena.jpg";
static const std::string MNIST = "Image-Folder/mnist/img_100.jpg";
static const std::string TILES = "Image-Folder/cifar10";

// 載入 lena 成 RGBImage、套一個單一 filter、dump 成檔。每次重新載入確保乾淨輸入。
static void dump_filter(uint16_t opt, const std::string &outfile) {
    RGBImage img;
    if (!img.LoadImage(LENA)) return;
    BitFieldFilter f;
    f.enable(opt);
    img.ApplyFilters(f);
    img.DumpImage(OUT + outfile);
}

int main() {
    system("mkdir -p report_assets");

    // ---------- Step 1 / Step 2：data_loader + 繼承多型的 Load/Dump/ASCII ----------
    // 灰階：透過 Image* 指標載入 mnist，dump 回檔（證明 load→pixel matrix→dump round-trip）
    {
        Image *g = new GrayImage();
        if (g->LoadImage(MNIST)) {
            g->DumpImage(OUT + "s2_gray_mnist.jpg");
            g->Display_ASCII();   // 印到 stdout，外層用 > 導到 ascii_art.txt
        }
        delete g;
    }
    // 彩色：透過 Image* 指標載入 lena，dump 回檔
    {
        Image *r = new RGBImage();
        if (r->LoadImage(LENA)) {
            r->DumpImage(OUT + "s2_rgb_lena.jpg");
        }
        delete r;
    }

    // ---------- Step 3：bit-field 四個基本 filter（+ 組合）----------
    dump_filter(BitFieldFilter::BLUR,     "s3_blur.jpg");
    dump_filter(BitFieldFilter::SOBEL,    "s3_sobel.jpg");
    dump_filter(BitFieldFilter::CONTRAST, "s3_contrast.jpg");
    dump_filter(BitFieldFilter::MOSAIC,   "s3_mosaic.jpg");
    dump_filter(BitFieldFilter::BLUR | BitFieldFilter::SOBEL, "s3_blur_sobel.jpg");

    // ---------- Step 4：Photo Mosaic（lena 由 cifar10 小圖拼成）----------
    {
        // 純 mosaic（alpha=0）與 alpha 混合（0.5）各一張，方便報告對照
        PhotoMosaic pure(8, 0.0);
        RGBImage *p = pure.generate(TILES, LENA);
        if (p) { p->DumpImage(OUT + "s4_photomosaic_pure.jpg"); delete p; }

        PhotoMosaic blended(8, 0.5);
        RGBImage *b = blended.generate(TILES, LENA);
        if (b) {
            b->DumpImage(OUT + "s4_photomosaic.jpg");
            // 放大局部，凸顯「小圖當像素」的結構
            RGBImage *zoom = b->crop(0, 0, 128, 128);
            if (zoom) { zoom->DumpImage(OUT + "s4_photomosaic_zoom.jpg"); delete zoom; }
            delete b;
        }
    }

    // ---------- Step 5：額外 filter + 單圖 mosaic + Factory ----------
    dump_filter(BitFieldFilter::BRIGHTNESS, "s5_brightness.jpg");
    dump_filter(BitFieldFilter::SHARPEN,    "s5_sharpen.jpg");
    dump_filter(BitFieldFilter::MEDIAN,     "s5_median.jpg");
    dump_filter(BitFieldFilter::INVERT,     "s5_invert.jpg");
    dump_filter(BitFieldFilter::GRAYSCALE,  "s5_grayscale.jpg");
    dump_filter(BitFieldFilter::THRESHOLD,  "s5_threshold.jpg");

    {
        PhotoMosaic single(8);
        RGBImage *sr = single.generate_single(LENA, LENA);
        if (sr) { sr->DumpImage(OUT + "s5_mosaic_single.jpg"); delete sr; }
    }

    {
        Image *fr = ImageFactory::create(LENA);
        if (fr) { fr->DumpImage(OUT + "s5_factory_rgb.jpg"); delete fr; }
        Image *fg = ImageFactory::create(MNIST);
        if (fg) { fg->DumpImage(OUT + "s5_factory_gray.jpg"); delete fg; }
    }

    return 0;
}
