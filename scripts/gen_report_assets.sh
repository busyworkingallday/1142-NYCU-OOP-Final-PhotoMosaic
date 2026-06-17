#!/bin/bash
# 在 Linux/Docker 容器內執行：產生 final_report.pdf 所需的各 Step 結果圖。
# 用法：在專案根目錄下  bash scripts/gen_report_assets.sh
set -e

# 0. 確認第三方相依已安裝（若 third-party 已存在會很快略過）
if [ ! -d third-party/CImg ] || [ ! -d third-party/libjpeg ]; then
    echo "[*] 安裝第三方相依 (make install)..."
    make install
fi

# 1. 編譯各 .o（main 不需要，但 obj/*.o 要齊）
echo "[*] 編譯 obj/*.o (make -j)..."
make -j

# 2. 編譯報告用 driver
echo "[*] 編譯 report_assets..."
mkdir -p report_assets
g++ -I ./inc -I ./third-party/CImg -I ./third-party/libjpeg -I ./Data-Loader \
    -std=c++11 -O3 report_assets.cpp \
    obj/bit_field_filter.o obj/data_loader.o obj/gray_image.o obj/image.o \
    obj/image_factory.o obj/photo_mosaic.o obj/rgb_image.o \
    -o report_assets -L./third-party/libjpeg -ljpeg -lpng -lm -lpthread

# 3. 執行，產生所有結果圖；ASCII art 導到文字檔
echo "[*] 產生結果圖到 report_assets/ ..."
./report_assets > report_assets/ascii_art.txt

echo ""
echo "[完成] 產出檔案："
ls -1 report_assets/
echo ""
echo "請把整個 report_assets/ 資料夾交回，即可嵌進 final_report.pdf。"
