/*
Riff file implementation
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

#include "riff.hh"

riff::chunk *riff::GetChunk(const key Length, const void *Data)
{
  if (Length < sizeof(riff::chunk))
  {
    return 0x0;
  }

  riff::chunk *Result = (riff::chunk *)Data;

  if (riff::IsRiffChunk(Result))
  {
    return Result;
  }

  return 0x0;
}

riff::iterator riff::GetIterator(const riff::chunk *Chunk)
{
  byte *ChunkData = ((byte *)Chunk) + sizeof(riff::chunk);
  riff::iterator Iterator = {
      .Current = ChunkData,
      .End = ChunkData + Chunk->ChunkSize - 4, // Remove 4 bytes because FormatType is part of Data
  };

  return Iterator;
}

riff::iterator riff::NextSubChunk(const riff::iterator Iterator)
{
  riff::sub_chunk *SubChunk = (riff::sub_chunk *)Iterator.Current;
  u32 Size = (SubChunk->ChunkSize + 1) & ~1;

  return {
      .Current = Iterator.Current + Size + sizeof(riff::sub_chunk),
      .End = Iterator.End,
  };
}

u32 riff::GetChunkID(const riff::iterator Iterator)
{
  riff::sub_chunk *SubChunk = (riff::sub_chunk *)Iterator.Current;
  return SubChunk->ChunkID;
}

u32 riff::GetChunkSize(const riff::iterator Iterator)
{
  riff::sub_chunk *SubChunk = (riff::sub_chunk *)Iterator.Current;
  return SubChunk->ChunkSize;
}

void *riff::GetChunkData(const riff::iterator Iterator)
{
  return Iterator.Current + sizeof(riff::sub_chunk);
}

riff::iterator riff::GetIteratorByID(const riff::chunk *Chunk, const u32 ID)
{
  riff::iterator Iterator = riff::GetIterator(Chunk);

  while (!riff::EndOfChunk(Iterator))
  {
    if (riff::GetChunkID(Iterator) == ID)
    {
      return Iterator;
    }

    Iterator = riff::NextSubChunk(Iterator);
  }

  return riff::ZeroIterator();
}
