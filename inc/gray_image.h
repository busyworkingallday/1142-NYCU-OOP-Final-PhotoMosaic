#ifndef _GRAY_IMAGE_H_
#define _GRAY_IMAGE_H_

#include "image.h"

class GrayImage : public Image{
private:
    int **pixels;

    void release();   // free pixels + reset dims; shared by dtor and LoadImage

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
};

#endif
