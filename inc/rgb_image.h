#ifndef _RGB_IMAGE_H_
#define _RGB_IMAGE_H_

#include "image.h"

class RGBImage : public Image{
    friend class BitFieldFilter;   // Step 3: reads/replaces pixels to apply filters
    friend class PhotoMosaic;      // Step 4: reads pixels to build the mosaic

private:
    int ***pixels;

    void release();   // free pixels + reset dims; shared by dtor and LoadImage
    void replace_buffer(int ***new_pixels);   // Step 3: adopt a new same-size buffer, free the old

public:
    RGBImage();
    // Takes ownership of `pixels`: freed by this object's destructor.
    // Caller must not free it afterwards nor share it with another image.
    RGBImage(int w, int h, int ***pixels);
    ~RGBImage();

    // take ownership + raw pointer => default shallow copy would double-free
    RGBImage(const RGBImage &) = delete;
    RGBImage &operator=(const RGBImage &) = delete;

    bool LoadImage(string filename) override;
    void DumpImage(string filename) override;
    void Display_ASCII() override;
    void Display_CMD() override;

    void ApplyFilters(const BitFieldFilter &filter) override;   // Step 3 (double dispatch)

    void resize(int newW, int newH);   // Step 4: nearest-neighbour rescale (changes dims)
};

#endif
