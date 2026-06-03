#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "data_loader.h"   // provides Data_Loader and `using namespace std;`

class Image{
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
};

#endif
