/*
WAV file implementation
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

#include "wav.hh"

riff::chunk *wav::GetChunk(buffer Buffer)
{
  riff::chunk *RiffChunk = riff::GetChunk(Buffer);

  if (!RiffChunk)
  {
    return 0x0;
  }

  if (!wav::IsWaveFormatType(RiffChunk))
  {
    return 0x0;
  }

  return RiffChunk;
}

wav::format *wav::GetFormat(buffer Buffer)
{
  riff::chunk *Chunk = wav::GetChunk(Buffer);

  if (!Chunk)
  {
    return 0x0;
  }

  riff::iterator Iterator = riff::GetIteratorByID(Chunk, wav::CHUNKID_FORMAT);

  if (riff::EndOfChunk(Iterator))
  {
    return 0x0;
  }

  return (wav::format *)riff::GetChunkData(Iterator);
}

wav::audio wav::GetAudio(buffer Buffer)
{
  riff::chunk *Chunk = wav::GetChunk(Buffer);

  if (!Chunk)
  {
    return wav::ZeroAudio();
  }

  riff::iterator Iterator = riff::GetIterator(Chunk);

  if (riff::EndOfChunk(Iterator))
  {
    return wav::ZeroAudio();
  }

  wav::format *Format = (wav::format *)riff::GetChunkData(Iterator);

  Iterator = riff::NextSubChunk(Iterator);

  if (riff::EndOfChunk(Iterator))
  {
    return wav::ZeroAudio();
  }

  wav::data *Data = (wav::data *)riff::GetChunkData(Iterator);
  u32 Size = riff::GetChunkSize(Iterator);

  return {
      .SampleCount = Size / (Format->BitsPerSample / 8),
      .ChannelCount = Format->Channels,
      .SampleData = Data,
  };
}
