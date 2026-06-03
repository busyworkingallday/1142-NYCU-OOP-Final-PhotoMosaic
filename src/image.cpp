#include "image.h"

// The single definition of the shared loader (static storage duration).
Data_Loader Image::data_loader;

Image::Image() : width(0), height(0) {}

Image::~Image() {}

int Image::get_w() const { return width; }

int Image::get_h() const { return height; }
