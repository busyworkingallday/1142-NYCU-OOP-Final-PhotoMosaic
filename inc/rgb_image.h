#ifndef _RGB_IMAGE_H_
#define _RGB_IMAGE_H_

#include "image.h"

class RGBImage : public Image{
private:
    int ***pixels;

    void release();   // free pixels + reset dims; shared by dtor and LoadImage

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
};

#endif
