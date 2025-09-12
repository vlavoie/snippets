#pragma once

#include <common.hh>
#include "texture.hh"

namespace atlas
{
struct pack
{
  texture *Texture;
};

inline atlas::pack CreateAtlas(key Width, key Height)
{
  return {
      .Texture = CreateEmptyTexture(Width, Height),
  };
}

inline void InsertTexture(atlas::pack *Atlas, texture *Texture)
{
  Assert(Texture->Width <= Atlas->Texture->Width && Texture->Height <= Atlas->Texture->Height,
         "Texture cannot fit in atlas.");

  for (key Y = 0; Y < Texture->Height; Y++)
  {
    for (key X = 0; X < Texture->Width; X++)
    {
      WritePixelAt(Atlas->Texture, X, Y, GetPixel(Texture, X, Y));
    }
  }
}
} // namespace atlas
