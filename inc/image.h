#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "data_loader.h"   // provides Data_Loader and `using namespace std;`

class BitFieldFilter;   // 前向宣告
class PhotoMosaic;      // 前向宣告(Step 4)

class Image{
    friend class PhotoMosaic;   // Step 4: uses the shared data_loader to list the tile folder
protected:
    int width;
    int height;
    static Data_Loader data_loader;   // shared across all images (declaration only)

public:
    Image();
    virtual ~Image();                 // virtual: correct destruction through Image*

    int get_w() const;
    int get_h() const;

    // pure virtual interface — dynamic binding through Image*
    virtual bool LoadImage(string filename) = 0;
    virtual void DumpImage(string filename) = 0;
    virtual void Display_ASCII() = 0;
    virtual void Display_CMD() = 0;

    // Step 3: apply a bit-field filter set to this image (double dispatch —
    // each derived class calls filter.RunOn(*this) with its concrete type).
    virtual void ApplyFilters(const BitFieldFilter &filter) = 0;
};

#endif
