#ifndef _GRAY_IMAGE_H_
#define _GRAY_IMAGE_H_

#include "image.h"

class GrayImage : public Image{
    friend class BitFieldFilter;   // Step 3: reads/replaces pixels to apply filters

private:
    int **pixels;

    void release();   // free pixels + reset dims; shared by dtor and LoadImage
    void replace_buffer(int **new_pixels);   // Step 3: adopt a new same-size buffer, free the old

public:
    GrayImage();
    // Takes ownership of `pixels`: freed by this object's destructor.
    // Caller must not free it afterwards nor share it with another image.
    GrayImage(int w, int h, int **pixels);
    ~GrayImage();

    // take ownership + raw pointer => default shallow copy would double-free
    GrayImage(const GrayImage &) = delete;
    GrayImage &operator=(const GrayImage &) = delete;

    bool LoadImage(string filename) override;
    void DumpImage(string filename) override;
    void Display_ASCII() override;
    void Display_CMD() override;

    void ApplyFilters(const BitFieldFilter &filter) override;   // Step 3 (double dispatch)
};

#endif
