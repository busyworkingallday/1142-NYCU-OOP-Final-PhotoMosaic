#include "gray_image.h"
#include "temp_file_guard.h"   // TempFileGuard (RAII temp-file cleanup)
#include <cstdint>             // uintptr_t

GrayImage::GrayImage() : pixels(nullptr) {}   // base Image() sets width/height = 0

// Takes ownership of `pixels`: stored as-is, freed by this object's destructor.
// Caller must not free it afterwards nor share it with another image.
GrayImage::GrayImage(int w, int h, int **pixels) : pixels(pixels) {
    if (pixels == nullptr) {
        width = 0;
        height = 0;
    } else {
        width = w;
        height = h;
    }
}

GrayImage::~GrayImage() {
    release();
}

// Invariant: pixels != nullptr  <=>  width > 0 && height > 0.
// Frees each row (new int[w]) then the row array (new int*[h]).
void GrayImage::release() {
    if (pixels != nullptr) {
        for (int y = 0; y < height; ++y)
            delete [] pixels[y];
        delete [] pixels;
        pixels = nullptr;
    }
    width = 0;
    height = 0;
}

bool GrayImage::LoadImage(string filename) {
    release();   // drop any previously held buffer first (reload-safe, leak-free)

    // Probe with CImg before touching Data_Loader (data_loader.cpp is left as-is):
    //  - a missing/unreadable file throws CImgException here, so we return false
    //    instead of hitting Load_Gray's assert(File_Exists(...)) and aborting;
    //  - a channel count other than 1/3/4 would make Load_Gray allocate its int**
    //    and then return nullptr (a leak in the provided loader), so we bail out
    //    before calling it.
    try {
        CImg<unsigned char> probe(filename.c_str());
        int c = probe.spectrum();
        if (c != 1 && c != 3 && c != 4)
            return false;
    } catch (const CImgException &) {
        return false;
    }

    int w = 0, h = 0;
    int **p = data_loader.Load_Gray(filename, &w, &h);
    if (p == nullptr)
        return false;   // dims stay 0, pixels stays nullptr

    pixels = p;
    width = w;
    height = h;
    return true;
}

void GrayImage::DumpImage(string filename) {
    if (pixels == nullptr)
        return;
    data_loader.Dump_Gray(width, height, pixels, filename);
}

void GrayImage::Display_ASCII() {
    if (pixels == nullptr)
        return;
    data_loader.Display_Gray_ASCII(width, height, pixels);
}

// No backing file in memory: dump to a per-object temp file, display it, then
// remove it. The object's address makes the name unique so two images
// displaying at the same time never clash on one temp file. std::remove is
// portable and needs no return-value check (unlike system()).
void GrayImage::Display_CMD() {
    if (pixels == nullptr)
        return;
    string tmp = "._tmp_gray_" + to_string(reinterpret_cast<uintptr_t>(this)) + ".jpg";
    TempFileGuard guard(tmp);   // removes tmp on scope exit, even if the below throws
    data_loader.Dump_Gray(width, height, pixels, tmp);
    data_loader.Display_Gray_CMD(tmp);
}
