#ifndef _PHOTO_MOSAIC_H_
#define _PHOTO_MOSAIC_H_

#include <string>

class RGBImage;   // 前向宣告:generate 回傳 RGBImage*

// Step 4: build a photo mosaic of a target image out of many small tile images.
// Integrates with the Image hierarchy (loads via RGBImage, friend of RGBImage to
// read pixels) and Data_Loader (enumerates the tile folder, loads each image).
class PhotoMosaic{
public:
    // cell_size: side length (px) of each grid cell; tiles are rescaled to this.
    explicit PhotoMosaic(int cell_size = 16);

    // Tile the target into cell_size x cell_size grids; for each grid pick the
    // tile whose per-channel RGB average minimises sqrt(rd^2 + gd^2 + bd^2), and
    // paste that tile (rescaled to the cell) into the output.
    // Returns a heap RGBImage* the caller owns and must delete; nullptr on
    // failure (target not loadable, or no usable RGB tile in tileDir).
    RGBImage *generate(const std::string &tileDir, const std::string &targetPath);

private:
    int cell_size;
};

#endif
