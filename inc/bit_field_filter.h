#ifndef _BIT_FIELD_FILTER_H_
#define _BIT_FIELD_FILTER_H_

#include <cstdint>

// Forward declarations: the double-dispatch targets. Full definitions are only
// needed in bit_field_filter.cpp, so the header stays free of image includes.
class GrayImage;
class RGBImage;

// Step 3: a bit-field-driven image filter.
//   - bitwise OR (|) loads options:   f.enable(BLUR | SOBEL);
//   - bitwise AND (&) tests an option: if (isEnabled(BLUR)) ...
// Applied to an image through the Image hierarchy via double dispatch:
//   img->ApplyFilters(filter)  ->  filter.RunOn(*this)  ->  RunOn(GrayImage&/RGBImage&)
//
// Note: the MOSAIC filter here pixelates/censors an image; it is unrelated to the
// Step 4 PhotoMosaic class (tile collage).
class BitFieldFilter {
public:
    // Option flags. C++11 enum with a fixed underlying type; values use 1 << n
    // (binary literals like 0b0001 are C++14 and the project builds with -std=c++11).
    enum Option : uint8_t {
        BLUR     = 1 << 0,   // 0x01  3x3 box blur
        SOBEL    = 1 << 1,   // 0x02  Sobel edge detection (vertical + horizontal)
        CONTRAST = 1 << 2,   // 0x04  contrast stretching
        MOSAIC   = 1 << 3,   // 0x08  mosaic / pixelation
    };

    BitFieldFilter();                       // no options enabled (options = 0)

    void enable(uint8_t opt);               // options |= opt   (bitwise OR load)
    void disable(uint8_t opt);              // options &= ~opt
    bool isEnabled(uint8_t opt) const;      // (options & opt)  (bitwise AND test)
    uint8_t get_options() const;

    // Double-dispatch entry points, invoked by GrayImage/RGBImage::ApplyFilters.
    // Each applies every enabled filter to img's pixels, in the fixed order
    // BLUR -> SOBEL -> CONTRAST -> MOSAIC. BitFieldFilter is a friend of the
    // image classes, so it can read/replace their private pixel buffers.
    void RunOn(GrayImage &img) const;
    void RunOn(RGBImage  &img) const;

private:
    uint8_t options;
};

#endif
