#pragma once

#include <common.hh>

#ifndef __TGA__Allocate
#include <stdlib.h>
#define __TGA__Allocate(_N) malloc(_N)
#endif

namespace tga
{
enum error_code
{
  ERROR_CODE_SUCCESS = 0,
  ERROR_CODE_DATA_SIZE = 1,      // Data size was smaller than header
  ERROR_CODE_DATA_TYPE = 2,      // Can only handle image type 1, 2 and 10
  ERROR_CODE_COLOR_MAP_TYPE = 3, // Can only handle color map types of 0 and 1
  ERROR_CODE_PIXEL_DEPTH = 4,    // Can only handle pixel depths of 8, 16, 24, and 32
};

struct pixel
{
  byte R, G, B, A;
};

struct reader
{
  byte *Offset;
};

struct texture
{
  u32 Width;
  u32 Height;
  tga::pixel *Pixels;
};

struct header
{
  byte IdLength;
  byte ColorMapType;
  byte DataTypeCode;
  word ColorMapOrigin;
  word ColorMapLength;
  byte ColorMapDepth;
  word OriginX;
  word OriginY;
  word Width;
  word Height;
  byte PixelDepth;
  byte ImageDescriptor;
};

tga::texture *Decompress(key Length, void *Data);
tga::texture *Decompress(key Length, void *Data, i32 *ErrorCode);
}; // namespace tga
