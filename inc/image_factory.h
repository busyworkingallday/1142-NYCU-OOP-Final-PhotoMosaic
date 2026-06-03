#ifndef _IMAGE_FACTORY_H_
#define _IMAGE_FACTORY_H_

#include <string>

class Image;

// Step 5: Factory Pattern. Inspects an image file's channel count and creates the
// matching concrete type (GrayImage for 1 channel, RGBImage for 3+), returned as a
// base Image* so callers stay decoupled from the concrete classes (polymorphism).
class ImageFactory{
public:
    // Returns a heap Image* the caller owns and must delete; nullptr if the file
    // cannot be read or loaded.
    static Image *create(const std::string &filename);
};

#endif
