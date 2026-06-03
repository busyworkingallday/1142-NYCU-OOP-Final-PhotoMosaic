# Photo Mosaic — OOP Final Project

## Build & run
- `make install` once to fetch CImg / libjpeg / catimg.
- `make -j` to build -> produces `./Image_Processing`.
- `make check` runs valgrind. Grading checks for memory leaks: zero tolerance.

## Provided vs. mine
- PROVIDED (do NOT reimplement): `Data-Loader/data_loader.{h,cpp}`. Use its interface.
  See `data_loader_demo.cpp` for working usage examples.
- MINE (implement): headers in `inc/` (currently empty stubs) and sources in `src/`
  (currently empty): image, gray_image, rgb_image, bit_field_filter, photo_mosaic.
  Plus flesh out the `main.cpp` driver.

## Hard requirements (rubric)
- Step 2: `Image` is an abstract base. `GrayImage` / `RGBImage` inherit publicly.
  `LoadImage` / `DumpImage` / `Display_ASCII` / `Display_CMD` are pure virtual in
  `Image` and overridden in the derived classes (use the `override` keyword).
  Dynamic binding must work through `Image*` pointers (no object slicing).
  `Image` has a virtual destructor.
- `width` / `height` / `data_loader` are protected in `Image`. Pixel arrays are
  private in the derived classes.
- `Data_Loader` is SHARED across all images via a static member or Singleton,
  not constructed once per object.
- Pixels are raw `int**` (gray) / `int***` (rgb): every `new[]` needs a matching
  `delete[]`. Must pass valgrind with no leaks and no invalid reads/writes.
- Step 3: at least 4 image filters, toggled via a bit field (bitwise OR to set an
  option, bitwise AND to test it).
- Step 4: a `PhotoMosaic` class. For each grid in the target image, pick the tile
  image whose per-channel RGB averages minimize sqrt(rdiff^2 + gdiff^2 + bdiff^2).

## Conventions
- C++11 (Makefile uses -std=c++11).
- One class per file in `src/`, matching the Makefile's `wildcard src/*.cpp`.
- Keep `Data_Loader`'s dynamically allocated returns owned and freed by my classes.
