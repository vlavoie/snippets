/*
TGA file header
Copyright (C) 2025  Vincent Lavoie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <common.hh>
#include "texture.hh"

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

struct reader
{
  const byte *Offset;
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

texture *Decompress(const key Length, const void *Data);
texture *Decompress(const key Length, const void *Data, i32 *ErrorCode);
}; // namespace tga
