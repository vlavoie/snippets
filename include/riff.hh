/*
Riff file header
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

namespace riff
{
struct chunk
{
  u32 ChunkID;
  u32 ChunkSize;
  u32 FormatType;
};

struct sub_chunk
{
  u32 ChunkID;
  u32 ChunkSize;
};

struct iterator
{
  byte *Current;
  byte *End;
};

enum
{
  FORMAT_TYPE = FourCC('R', 'I', 'F', 'F'),
};

inline riff::iterator ZeroIterator()
{
  return {
      .Current = 0x0,
      .End = 0x0,
  };
}

inline bool32 IsRiffChunk(riff::chunk *Chunk)
{
  return Chunk->ChunkID == riff::FORMAT_TYPE;
};

inline bool32 EndOfChunk(riff::iterator Iterator)
{
  return Iterator.Current >= Iterator.End;
}

riff::chunk *GetChunk(buffer Buffer);
riff::iterator GetIterator(riff::chunk *Chunk);
riff::iterator GetIteratorByID(riff::chunk *Chunk, u32 ID);
riff::iterator NextSubChunk(riff::iterator Iterator);
u32 GetChunkID(riff::iterator Iterator);
u32 GetChunkSize(riff::iterator Iterator);
void *GetChunkData(riff::iterator Iterator);
} // namespace riff
