/*
Header for textures and utils
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

struct pixel
{
  byte R, G, B, A;
};

struct texture
{
  key Width, Height;
  pixel *Pixels;
};

inline texture *CreateEmptyTexture(key Width, key Height)
{
  texture *Result = SysAllocate(texture, 1);
  Result->Width = Width;
  Result->Height = Height;
  Result->Pixels = SysAllocate(pixel, Width * Height);
  return Result;
};

constexpr inline pixel *PixelAt(texture *Texture, key X, key Y)
{
  Assert(Texture->Width > X && Texture->Height > Y, "Pixel index was out of range.");
  return &Texture->Pixels[Texture->Width * Y + X];
}

constexpr inline pixel GetPixel(texture *Texture, key X, key Y)
{
  Assert(Texture->Width > X && Texture->Height > Y, "Pixel index was out of range.");
  return Texture->Pixels[Texture->Width * Y + X];
}

constexpr inline void WritePixelAt(texture *Texture, key X, key Y, pixel R)
{
  pixel *Pixel = PixelAt(Texture, X, Y);
  Pixel->R = R.R;
  Pixel->G = R.G;
  Pixel->B = R.B;
  Pixel->A = R.A;
}
