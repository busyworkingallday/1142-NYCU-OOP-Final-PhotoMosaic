#include "rgb_image.h"
#include "temp_file_guard.h"   // TempFileGuard (RAII temp-file cleanup)
#include <cstdint>             // uintptr_t

RGBImage::RGBImage() : pixels(nullptr) {}   // base Image() sets width/height = 0

// Takes ownership of `pixels`: stored as-is, freed by this object's destructor.
// Caller must not free it afterwards nor share it with another image.
RGBImage::RGBImage(int w, int h, int ***pixels) : pixels(pixels) {
    if (pixels == nullptr) {
        width = 0;
        height = 0;
    } else {
        width = w;
        height = h;
    }
}

RGBImage::~RGBImage() {
    release();
}

// Invariant: pixels != nullptr  <=>  width > 0 && height > 0.
// Triple-nested free, innermost first:
//   per-pixel int[3]  ->  per-row int*[w]  ->  top int**[h].
void RGBImage::release() {
    if (pixels != nullptr) {
        for (int y = 0; y < height; ++y) {
            if (pixels[y] != nullptr) {
                for (int x = 0; x < width; ++x)
                    delete [] pixels[y][x];
                delete [] pixels[y];
            }
        }
        delete [] pixels;
        pixels = nullptr;
    }
    width = 0;
    height = 0;
}

bool RGBImage::LoadImage(string filename) {
    release();   // drop any previously held buffer first (reload-safe, leak-free)

    // Probe with CImg before touching Data_Loader (data_loader.cpp is left as-is):
    // a missing/unreadable file throws CImgException here, so we return false
    // instead of hitting Load_RGB's assert(File_Exists(...)) and aborting.
    // No spectrum guard is needed: Load_RGB cleanly returns nullptr (no leak)
    // for images with < 3 channels.
    try {
        CImg<unsigned char> probe(filename.c_str());
        (void)probe;
    } catch (const CImgException &) {
        return false;
    }

    int w = 0, h = 0;
    int ***p = data_loader.Load_RGB(filename, &w, &h);
    if (p == nullptr)
        return false;   // e.g. image has < 3 channels; dims/pixels stay empty

    pixels = p;
    width = w;
    height = h;
    return true;
}

void RGBImage::DumpImage(string filename) {
    if (pixels == nullptr)
        return;
    data_loader.Dump_RGB(width, height, pixels, filename);
}

void RGBImage::Display_ASCII() {
    if (pixels == nullptr)
        return;
    data_loader.Display_RGB_ASCII(width, height, pixels);
}

// No backing file in memory: dump to a per-object temp file, display it, then
// remove it. The object's address makes the name unique so two images
// displaying at the same time never clash on one temp file. std::remove is
// portable and needs no return-value check (unlike system()).
void RGBImage::Display_CMD() {
    if (pixels == nullptr)
        return;
    string tmp = "._tmp_rgb_" + to_string(reinterpret_cast<uintptr_t>(this)) + ".jpg";
    TempFileGuard guard(tmp);   // removes tmp on scope exit, even if the below throws
    data_loader.Dump_RGB(width, height, pixels, tmp);
    data_loader.Display_RGB_CMD(tmp);
}
