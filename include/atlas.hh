/*
Header for creating texture atlases
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

#include "common.hh"
#include "math2d.hh"
#include "texture.hh"

namespace atlas
{
struct pack
{
  key IndexX;
  key IndexY;
  texture *Texture;
};

struct coordinates
{
  vec2 Start;
  vec2 End;
};

inline atlas::pack CreateAtlas(key Width, key Height)
{
  return {
      .IndexX = 0,
      .IndexY = 0,
      .Texture = CreateEmptyTexture(Width, Height),
  };
}

inline void NextRow(atlas::pack *Atlas, key Height)
{
  Atlas->IndexX = 0;
  Atlas->IndexY += Height;
}

inline vec2 ToCoordinates(atlas::pack *Atlas, vec2 Position)
{
  return {
      .X = Position.X / f32(Atlas->Texture->Width),
      .Y = Position.Y / f32(Atlas->Texture->Height),
  };
}

inline atlas::coordinates InsertTexture(atlas::pack *Atlas, texture *Texture)
{
  Assert(Texture->Width <= Atlas->Texture->Width && Texture->Height <= Atlas->Texture->Height,
         "Texture cannot fit in atlas.");

  atlas::coordinates Result;

  if (Atlas->IndexX + Texture->Width > Atlas->Texture->Width)
  {
    NextRow(Atlas, Texture->Height);
  }

  for (key Y = 0; Y < Texture->Height; Y++)
  {
    for (key X = 0; X < Texture->Width; X++)
    {
      WritePixelAt(Atlas->Texture, X + Atlas->IndexX, Y + Atlas->IndexY, GetPixel(Texture, X, Y));
    }
  }

  Result.Start =
      atlas::ToCoordinates(Atlas, vec2{.X = f32(Atlas->IndexX), .Y = f32(Atlas->IndexY)});
  Result.End = atlas::ToCoordinates(Atlas, vec2{.X = f32(Atlas->IndexX + Texture->Width),
                                                .Y = f32(Atlas->IndexY + Texture->Height)});

  Atlas->IndexX += Texture->Width;
  return Result;
}
} // namespace atlas
