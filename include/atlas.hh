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
struct coordinates
{
  vec2 Start;
  vec2 End;
};

struct pack
{
  key Size;

  atlas::coordinates *Coordinates;
  texture *Texture;
};

inline vec2 ToCoordinates(atlas::pack *Atlas, vec2 Position)
{
  return {
      .X = Position.X / f32(Atlas->Texture->Width),
      .Y = Position.Y / f32(Atlas->Texture->Height),
  };
}

inline atlas::pack *CreateAtlas(key TextureCount, texture **Textures)
{
  atlas::pack *Result = SysAllocate(atlas::pack, 1);
  Result->Size = TextureCount;
  Result->Coordinates = SysAllocate(atlas::coordinates, TextureCount);

  key TotalSize = 0;
  for (key TextureIndex = 0; TextureIndex < TextureCount; TextureIndex++)
  {
    texture *Texture = Textures[TextureIndex];
    TotalSize += Texture->Width * Texture->Height;
  }

  key PowerOfTwo = 2;

  while (PowerOfTwo * PowerOfTwo < TotalSize * 2)
  {
    PowerOfTwo *= 2;
  }

  Result->Texture = CreateEmptyTexture(PowerOfTwo, PowerOfTwo);

  key IndexX = 0, IndexY = 0, RowTallest = 0;

  for (key TextureIndex = 0; TextureIndex < TextureCount; TextureIndex++)
  {
    texture *Texture = Textures[TextureIndex];

    if (IndexX + Texture->Width > Result->Texture->Width)
    {
      IndexX = 0;
      IndexY += RowTallest;
      RowTallest = 0;
    }

    for (key Y = 0; Y < Texture->Height; Y++)
    {
      for (key X = 0; X < Texture->Width; X++)
      {
        WritePixelAt(Result->Texture, X + IndexX, Y + IndexY, GetPixel(Texture, X, Y));
      }
    }

    Result->Coordinates[TextureIndex].Start =
        atlas::ToCoordinates(Result, vec2{.X = f32(IndexX), .Y = f32(IndexY)});
    Result->Coordinates[TextureIndex].End = atlas::ToCoordinates(
        Result, vec2{.X = f32(IndexX + Texture->Width), .Y = f32(IndexY + Texture->Height)});

    RowTallest = Texture->Height > RowTallest ? Texture->Height : RowTallest;
    IndexX += Texture->Width;
  }

  return Result;
}
} // namespace atlas
