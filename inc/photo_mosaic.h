#ifndef _PHOTO_MOSAIC_H_
#define _PHOTO_MOSAIC_H_

#include <string>
#include <vector>

class RGBImage;   // 前向宣告:generate 回傳 RGBImage*

// Step 4: build a photo mosaic of a target image out of many small tile images.
// Integrates with the Image hierarchy (loads via RGBImage, friend of RGBImage to
// read pixels) and Data_Loader (enumerates the tile folder, loads each image).
class PhotoMosaic{
public:
    // cell_size: side length (px) of each grid cell; tiles are rescaled to this.
    // alpha: opacity of the original target re-blended over the finished mosaic
    //   (classic photomosaic trick to bring the target's outline/contrast back).
    //   0.0 = pure mosaic, 1.0 = pure original; default 0.35 (~35% original).
    explicit PhotoMosaic(int cell_size = 8, double alpha = 0.35);

    // Tune the alpha-blend strength after construction (clamped to [0, 1]).
    // Lets main.cpp compare effects without touching the constructor call.
    void set_alpha(double a);

    // Tile the target into cell_size x cell_size grids; for each grid pick the
    // tile whose per-channel RGB average minimises sqrt(rd^2 + gd^2 + bd^2), and
    // paste that tile (rescaled to the cell) into the output.
    // Returns a heap RGBImage* the caller owns and must delete; nullptr on
    // failure (target not loadable, or no usable RGB tile in tileDir).
    RGBImage *generate(const std::string &tileDir, const std::string &targetPath);

    // Step 5 — single-image mosaic (IEEE-style): the tile pool is the set of
    // distinct cell_size x cell_size blocks cropped from ONE source image, instead
    // of many separate files. Same nearest-colour selection rebuilds the target.
    // Returns a heap RGBImage* the caller owns and must delete; nullptr on failure
    // (target/source not loadable, or source smaller than one cell).
    RGBImage *generate_single(const std::string &sourcePath, const std::string &targetPath);

private:
    int cell_size;
    double alpha;     // original-target re-blend opacity; see set_alpha / blend step

    // Shared back end for both modes. Given a target and a pool of preloaded tiles
    // (each already cell_size x cell_size) with their per-channel average colours,
    // tile the target into whole cells and paste the nearest tile (min squared
    // RGB-average distance) into each. TAKES OWNERSHIP of every RGBImage* in
    // `tiles`: frees them all before returning, on success AND on every failure
    // path. Returns the assembled RGBImage* (caller owns) or nullptr.
    RGBImage *assemble(RGBImage &target,
                       std::vector<RGBImage*> &tiles,
                       std::vector<int> &avgR,
                       std::vector<int> &avgG,
                       std::vector<int> &avgB);
};

#endif
