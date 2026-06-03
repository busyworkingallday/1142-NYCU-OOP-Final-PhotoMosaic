#include "image_factory.h"
#include "image.h"          // brings in CImg / CImgException via data_loader.h
#include "gray_image.h"
#include "rgb_image.h"

// Probe the file's channel count with CImg (same approach as the LoadImage guards),
// pick the concrete image type, load it, and hand it back through an Image* so the
// caller never names GrayImage / RGBImage directly.
Image *ImageFactory::create(const std::string &filename) {
    int channels = 0;
    try {
        CImg<unsigned char> probe(filename.c_str());
        channels = probe.spectrum();
    } catch (const CImgException &) {
        return nullptr;     // unreadable / not an image
    }

    Image *img = (channels >= 3) ? static_cast<Image *>(new RGBImage())
                                 : static_cast<Image *>(new GrayImage());

    if (!img->LoadImage(filename)) {   // load via the chosen concrete type
        delete img;                    // virtual dtor -> correct cleanup
        return nullptr;
    }
    return img;
}
