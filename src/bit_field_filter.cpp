#include "bit_field_filter.h"
#include "gray_image.h"
#include "rgb_image.h"

// ===== bit-field management =====

BitFieldFilter::BitFieldFilter() : options(0) {}

void BitFieldFilter::enable(uint8_t opt)  { options |=  opt; }   // bitwise OR: load option
void BitFieldFilter::disable(uint8_t opt) { options &= ~opt; }   // clear option

bool BitFieldFilter::isEnabled(uint8_t opt) const {
    return (options & opt) != 0;                                 // bitwise AND: test option
}

uint8_t BitFieldFilter::get_options() const { return options; }

// ===== shared helpers =====

namespace {

// Clamp an intermediate value into the valid 8-bit pixel range [0, 255].
inline int clamp255(int v) {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return v;
}

// Clamp a coordinate into [lo, hi] (edge-replicate sampling at image borders).
inline int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Absolute value for int (kept local to avoid an extra <cstdlib>/<cmath> include).
inline int iabs(int v) { return v < 0 ? -v : v; }

// Sobel 3x3 kernels: horizontal (Gx) and vertical (Gy) gradients.
const int SOBEL_GX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const int SOBEL_GY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

// Mosaic block size in pixels (fixed 10x10).
const int MOSAIC_BLOCK = 10;

// ===== filter kernels =====
// BLUR and SOBEL are neighbourhood ops: they allocate a fresh same-size buffer,
// compute into it from the ORIGINAL pixels, and return it; RunOn hands the result
// to GrayImage/RGBImage::replace_buffer (which frees the old buffer). CONTRAST and
// MOSAIC are in-place: they modify the existing pixel buffer directly.

// 3x3 box blur. Neighbourhood op: cannot run in place (each output reads the
// ORIGINAL neighbours), so it allocates a fresh same-size buffer, computes into
// it from `src`, and returns it. RunOn hands the result to replace_buffer, which
// frees the old buffer. Borders use edge-replicate (clampi) so every pixel
// averages a full 3x3 window (sum / 9, always in range).
int **box_blur_gray(int **src, int w, int h) {
    int **out = new int*[h];
    for (int y = 0; y < h; ++y) {
        out[y] = new int[w];
        for (int x = 0; x < w; ++x) {
            int sum = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    sum += src[clampi(y + dy, 0, h - 1)][clampi(x + dx, 0, w - 1)];
            out[y][x] = sum / 9;
        }
    }
    return out;
}

int ***box_blur_rgb(int ***src, int w, int h) {
    int ***out = new int**[h];
    for (int y = 0; y < h; ++y) {
        out[y] = new int*[w];
        for (int x = 0; x < w; ++x) {
            out[y][x] = new int[3];
            for (int c = 0; c < 3; ++c) {
                int sum = 0;
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx)
                        sum += src[clampi(y + dy, 0, h - 1)][clampi(x + dx, 0, w - 1)][c];
                out[y][x][c] = sum / 9;
            }
        }
    }
    return out;
}

// Sobel edge detection. Like blur, a neighbourhood op: allocates a fresh buffer,
// computes from the ORIGINAL src, returns it (RunOn -> replace_buffer frees old).
// Per pixel: convolve the edge-replicated 3x3 window with Gx and Gy, output
// clamp255(|Gx| + |Gy|). RGB does this independently per channel.
int **sobel_gray(int **src, int w, int h) {
    int **out = new int*[h];
    for (int y = 0; y < h; ++y) {
        out[y] = new int[w];
        for (int x = 0; x < w; ++x) {
            int gx = 0, gy = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    int v = src[clampi(y + dy, 0, h - 1)][clampi(x + dx, 0, w - 1)];
                    gx += v * SOBEL_GX[dy + 1][dx + 1];
                    gy += v * SOBEL_GY[dy + 1][dx + 1];
                }
            out[y][x] = clamp255(iabs(gx) + iabs(gy));
        }
    }
    return out;
}

int ***sobel_rgb(int ***src, int w, int h) {
    int ***out = new int**[h];
    for (int y = 0; y < h; ++y) {
        out[y] = new int*[w];
        for (int x = 0; x < w; ++x) {
            out[y][x] = new int[3];
            for (int c = 0; c < 3; ++c) {
                int gx = 0, gy = 0;
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx) {
                        int v = src[clampi(y + dy, 0, h - 1)][clampi(x + dx, 0, w - 1)][c];
                        gx += v * SOBEL_GX[dy + 1][dx + 1];
                        gy += v * SOBEL_GY[dy + 1][dx + 1];
                    }
                out[y][x][c] = clamp255(iabs(gx) + iabs(gy));
            }
        }
    }
    return out;
}

// Contrast stretching (in place). Scan min/max, then linearly remap to [0,255]
// via (v-min)*255/(max-min). A flat channel (max==min) is left unchanged to
// avoid divide-by-zero. The mapping keeps results within [0,255] by construction.
void contrast_stretch_gray(int **pixels, int w, int h) {
    int mn = pixels[0][0], mx = pixels[0][0];
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int v = pixels[y][x];
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
    if (mx == mn) return;                       // flat: nothing to stretch
    int range = mx - mn;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            pixels[y][x] = (pixels[y][x] - mn) * 255 / range;
}

void contrast_stretch_rgb(int ***pixels, int w, int h) {
    for (int c = 0; c < 3; ++c) {               // each channel stretched independently
        int mn = pixels[0][0][c], mx = pixels[0][0][c];
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                int v = pixels[y][x][c];
                if (v < mn) mn = v;
                if (v > mx) mx = v;
            }
        if (mx == mn) continue;                 // flat channel: leave as-is
        int range = mx - mn;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                pixels[y][x][c] = (pixels[y][x][c] - mn) * 255 / range;
    }
}

// Mosaic / pixelation (in place). Tile the image into 10x10 blocks; replace each
// block with its own average. Blocks are disjoint, so computing then overwriting
// in place is safe. Partial edge blocks average only the pixels that exist
// (block extent clamped to w/h), so count >= 1 — no divide-by-zero.
void mosaic_gray(int **pixels, int w, int h) {
    for (int by = 0; by < h; by += MOSAIC_BLOCK) {
        for (int bx = 0; bx < w; bx += MOSAIC_BLOCK) {
            int y_end = (by + MOSAIC_BLOCK < h) ? by + MOSAIC_BLOCK : h;
            int x_end = (bx + MOSAIC_BLOCK < w) ? bx + MOSAIC_BLOCK : w;
            int sum = 0, count = 0;
            for (int y = by; y < y_end; ++y)
                for (int x = bx; x < x_end; ++x) { sum += pixels[y][x]; ++count; }
            int avg = sum / count;
            for (int y = by; y < y_end; ++y)
                for (int x = bx; x < x_end; ++x) pixels[y][x] = avg;
        }
    }
}

void mosaic_rgb(int ***pixels, int w, int h) {
    for (int by = 0; by < h; by += MOSAIC_BLOCK) {
        for (int bx = 0; bx < w; bx += MOSAIC_BLOCK) {
            int y_end = (by + MOSAIC_BLOCK < h) ? by + MOSAIC_BLOCK : h;
            int x_end = (bx + MOSAIC_BLOCK < w) ? bx + MOSAIC_BLOCK : w;
            for (int c = 0; c < 3; ++c) {
                int sum = 0, count = 0;
                for (int y = by; y < y_end; ++y)
                    for (int x = bx; x < x_end; ++x) { sum += pixels[y][x][c]; ++count; }
                int avg = sum / count;
                for (int y = by; y < y_end; ++y)
                    for (int x = bx; x < x_end; ++x) pixels[y][x][c] = avg;
            }
        }
    }
}

} // namespace

// ===== double-dispatch targets =====
// Apply every enabled filter, in the fixed order BLUR -> SOBEL -> CONTRAST -> MOSAIC.
// BitFieldFilter is a friend of the image classes, so it reads their private pixels.

void BitFieldFilter::RunOn(GrayImage &img) const {
    if (img.pixels == nullptr) return;          // not loaded: nothing to do
    int w = img.get_w();
    int h = img.get_h();
    if (isEnabled(BLUR))     img.replace_buffer(box_blur_gray(img.pixels, w, h));
    if (isEnabled(SOBEL))    img.replace_buffer(sobel_gray(img.pixels, w, h));
    if (isEnabled(CONTRAST)) contrast_stretch_gray(img.pixels, w, h);
    if (isEnabled(MOSAIC))   mosaic_gray(img.pixels, w, h);
}

void BitFieldFilter::RunOn(RGBImage &img) const {
    if (img.pixels == nullptr) return;          // not loaded: nothing to do
    int w = img.get_w();
    int h = img.get_h();
    if (isEnabled(BLUR))     img.replace_buffer(box_blur_rgb(img.pixels, w, h));
    if (isEnabled(SOBEL))    img.replace_buffer(sobel_rgb(img.pixels, w, h));
    if (isEnabled(CONTRAST)) contrast_stretch_rgb(img.pixels, w, h);
    if (isEnabled(MOSAIC))   mosaic_rgb(img.pixels, w, h);
}
