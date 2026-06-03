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
    // Underlying type is uint16_t because we now use bits up to 1 << 9 (> 8 bits).
    enum Option : uint16_t {
        BLUR       = 1 << 0,   // 0x001  3x3 box blur
        SOBEL      = 1 << 1,   // 0x002  Sobel edge detection (vertical + horizontal)
        CONTRAST   = 1 << 2,   // 0x004  contrast stretching
        MOSAIC     = 1 << 3,   // 0x008  mosaic / pixelation
        BRIGHTNESS = 1 << 4,   // 0x010  Step 5: per-pixel brightness +delta (in place)
        SHARPEN    = 1 << 5,   // 0x020  Step 5: 3x3 sharpen kernel (neighbourhood)
        MEDIAN     = 1 << 6,   // 0x040  Step 5: 3x3 median denoise (neighbourhood)
        INVERT     = 1 << 7,   // 0x080  Step 5: per-pixel 255 - v (in place)
        GRAYSCALE  = 1 << 8,   // 0x100  Step 5: RGB -> luma, set all channels (in place)
        THRESHOLD  = 1 << 9,   // 0x200  Step 5: v > 128 ? 255 : 0 (in place)
    };

    BitFieldFilter();                       // no options enabled (options = 0)

    void enable(uint16_t opt);              // options |= opt   (bitwise OR load)
    void disable(uint16_t opt);             // options &= ~opt
    bool isEnabled(uint16_t opt) const;     // (options & opt)  (bitwise AND test)
    uint16_t get_options() const;

    // Double-dispatch entry points, invoked by GrayImage/RGBImage::ApplyFilters.
    // Each applies every enabled filter to img's pixels, in the fixed order BLUR ->
    // SOBEL -> CONTRAST -> MOSAIC -> BRIGHTNESS -> SHARPEN -> MEDIAN -> INVERT ->
    // GRAYSCALE -> THRESHOLD. BitFieldFilter is a friend of the image classes, so
    // it can read/replace their private pixel buffers.
    void RunOn(GrayImage &img) const;
    void RunOn(RGBImage  &img) const;

private:
    uint16_t options;
};

#endif
