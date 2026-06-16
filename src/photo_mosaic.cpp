#include "photo_mosaic.h"
#include "image.h"
#include "rgb_image.h"
#include <vector>

PhotoMosaic::PhotoMosaic(int cell_size, double alpha)
    : cell_size(cell_size), alpha(alpha) {
    set_alpha(alpha);   // clamp the initial value into [0, 1]
}

void PhotoMosaic::set_alpha(double a) {
    if (a < 0.0) a = 0.0;
    if (a > 1.0) a = 1.0;
    alpha = a;
}

RGBImage *PhotoMosaic::generate(const std::string &tileDir, const std::string &targetPath) {
    if (cell_size <= 0) return nullptr;

    // 1. Load the target image (stack object: auto-frees on every return path).
    RGBImage target;
    if (!target.LoadImage(targetPath))
        return nullptr;                       // not RGB / missing -> fail cleanly

    // 2. Enumerate the tile folder and preload each usable tile.
    std::vector<std::string> files;
    Image::data_loader.List_Directory(tileDir, files);   // shared loader (friend access)

    std::vector<RGBImage*> tiles;             // owned here until handed off / freed
    std::vector<int> avgR, avgG, avgB;        // each tile's per-channel average colour

    for (size_t i = 0; i < files.size(); ++i) {
        RGBImage *tile = new RGBImage();
        if (!tile->LoadImage(files[i])) {     // grayscale / non-RGB tile -> skip
            delete tile;
            continue;
        }
        tile->resize(cell_size, cell_size);   // normalise every tile to one cell

        long sr = 0, sg = 0, sb = 0;          // friend access to private pixels
        for (int y = 0; y < cell_size; ++y)
            for (int x = 0; x < cell_size; ++x) {
                sr += tile->pixels[y][x][0];
                sg += tile->pixels[y][x][1];
                sb += tile->pixels[y][x][2];
            }
        int n = cell_size * cell_size;
        tiles.push_back(tile);
        avgR.push_back(static_cast<int>(sr / n));
        avgG.push_back(static_cast<int>(sg / n));
        avgB.push_back(static_cast<int>(sb / n));
    }

    // No usable RGB tile in the folder -> clean up and fail.
    if (tiles.empty())
        return nullptr;                       // target auto-destructs here

    // 3. Hand the preloaded tiles to the shared back end (it owns/frees them now).
    return assemble(target, tiles, avgR, avgG, avgB);
}

// Step 5 — single-image mosaic. The tile pool is the grid of cell_size x cell_size
// blocks cropped from ONE source image (RGBImage::crop, take-ownership), instead of
// many files. Reuses the same average + assemble back end as the multi-image mode.
RGBImage *PhotoMosaic::generate_single(const std::string &sourcePath, const std::string &targetPath) {
    if (cell_size <= 0) return nullptr;

    // Load target and source (both stack objects: auto-free on every return path).
    RGBImage target;
    if (!target.LoadImage(targetPath))
        return nullptr;
    RGBImage source;
    if (!source.LoadImage(sourcePath))
        return nullptr;                       // target auto-destructs here

    // Whole cell_size blocks of the source only (floor); a remainder strip on the
    // right/bottom smaller than one cell is dropped.
    int blocksX = source.get_w() / cell_size;
    int blocksY = source.get_h() / cell_size;
    if (blocksX == 0 || blocksY == 0)
        return nullptr;                       // source smaller than one cell

    std::vector<RGBImage*> tiles;             // owned here until handed to assemble
    std::vector<int> avgR, avgG, avgB;

    int n = cell_size * cell_size;
    for (int by = 0; by < blocksY; ++by) {
        for (int bx = 0; bx < blocksX; ++bx) {
            // Each block is exactly cell_size^2, so no resize is needed.
            RGBImage *tile = source.crop(bx * cell_size, by * cell_size, cell_size, cell_size);
            if (tile == nullptr) continue;    // defensive (in-bounds blocks won't hit this)

            long sr = 0, sg = 0, sb = 0;      // friend access to tile's private pixels
            for (int y = 0; y < cell_size; ++y)
                for (int x = 0; x < cell_size; ++x) {
                    sr += tile->pixels[y][x][0];
                    sg += tile->pixels[y][x][1];
                    sb += tile->pixels[y][x][2];
                }
            tiles.push_back(tile);
            avgR.push_back(static_cast<int>(sr / n));
            avgG.push_back(static_cast<int>(sg / n));
            avgB.push_back(static_cast<int>(sb / n));
        }
    }

    if (tiles.empty())
        return nullptr;                       // target & source auto-destruct here

    return assemble(target, tiles, avgR, avgG, avgB);
}

// Shared back end. Owns `tiles`: frees every entry before returning on ALL paths.
RGBImage *PhotoMosaic::assemble(RGBImage &target,
                                std::vector<RGBImage*> &tiles,
                                std::vector<int> &avgR,
                                std::vector<int> &avgG,
                                std::vector<int> &avgB) {
    // Partition the target into whole cells only (floor); a remainder strip on the
    // right/bottom smaller than one cell is dropped.
    int cellsX = target.get_w() / cell_size;
    int cellsY = target.get_h() / cell_size;
    if (cellsX == 0 || cellsY == 0) {         // target smaller than one cell
        for (size_t i = 0; i < tiles.size(); ++i) delete tiles[i];
        return nullptr;
    }
    int outW = cellsX * cell_size;
    int outH = cellsY * cell_size;

    // Allocate the output buffer in full BEFORE filling it (same int*** shape
    // Load_RGB uses, so ~RGBImage frees it correctly once we hand it over).
    int ***out = new int**[outH];
    for (int y = 0; y < outH; ++y) {
        out[y] = new int*[outW];
        for (int x = 0; x < outW; ++x)
            out[y][x] = new int[3];
    }

    // For each grid cell: average the target region, pick the closest tile by
    // squared RGB-average distance (argmin equals minimising the README sqrt), and
    // copy that tile's pixels into the cell's output block.
    int n = cell_size * cell_size;
    for (int gy = 0; gy < cellsY; ++gy) {
        for (int gx = 0; gx < cellsX; ++gx) {
            int oy = gy * cell_size;          // top-left of this cell in target/out
            int ox = gx * cell_size;

            long sr = 0, sg = 0, sb = 0;      // friend access to target's pixels
            for (int y = 0; y < cell_size; ++y)
                for (int x = 0; x < cell_size; ++x) {
                    sr += target.pixels[oy + y][ox + x][0];
                    sg += target.pixels[oy + y][ox + x][1];
                    sb += target.pixels[oy + y][ox + x][2];
                }
            int cr = static_cast<int>(sr / n);
            int cg = static_cast<int>(sg / n);
            int cb = static_cast<int>(sb / n);

            int best = 0;
            long best_d = -1;
            for (size_t t = 0; t < tiles.size(); ++t) {
                long dr = cr - avgR[t];
                long dg = cg - avgG[t];
                long db = cb - avgB[t];
                long d = dr * dr + dg * dg + db * db;
                if (best_d < 0 || d < best_d) {
                    best_d = d;
                    best = static_cast<int>(t);
                }
            }

            int ***src = tiles[best]->pixels;   // chosen tile, already cell_size^2
            for (int y = 0; y < cell_size; ++y)
                for (int x = 0; x < cell_size; ++x)
                    for (int c = 0; c < 3; ++c)
                        out[oy + y][ox + x][c] = src[y][x][c];
        }
    }

    // Alpha-blend the original target back over the finished mosaic so its outline
    // and light/dark contrast re-emerge while the tile texture is kept. Skip the
    // whole pass when alpha == 0 (pure mosaic). We resize `target` to the exact
    // output size so the two images line up pixel-for-pixel; the remainder strip
    // dropped above means outW/outH <= target's original dims, so this only ever
    // shrinks. resize() manages target's own buffer (replace_buffer), no leak here.
    if (alpha > 0.0) {
        target.resize(outW, outH);
        for (int y = 0; y < outH; ++y)
            for (int x = 0; x < outW; ++x)
                for (int c = 0; c < 3; ++c) {
                    double v = (1.0 - alpha) * out[y][x][c]
                             + alpha * target.pixels[y][x][c];
                    int iv = static_cast<int>(v + 0.5);   // round, then clamp 0..255
                    if (iv < 0) iv = 0;
                    if (iv > 255) iv = 255;
                    out[y][x][c] = iv;
                }
    }

    // Output fully filled. Free the tiles FIRST (we're done reading them), then
    // hand `out` to an RGBImage (take-ownership) and return it (caller owns it).
    // Freeing before the allocation keeps the tiles from leaking even if `new`
    // were to throw.
    for (size_t i = 0; i < tiles.size(); ++i)
        delete tiles[i];
    return new RGBImage(outW, outH, out);
}
