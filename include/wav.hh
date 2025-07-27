/*
WAV file header
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
#include "riff.hh"

namespace wav
{
typedef short sample;

enum
{
  CHUNKID_FORMAT = FourCC('f', 'm', 't', ' '),
  CHUNKID_DATA = FourCC('d', 'a', 't', 'a'),
  CHUNKID_WAVE = FourCC('W', 'A', 'V', 'E'),
};

struct format
{
  word FormatCode;
  word Channels;
  u32 SamplesPerSecond;
  u32 AverageBytesPerSecond;
  word BlockAlign;
  word BitsPerSample;
  word ExtensionSize;
  word ValidBitsPerSample;
  u32 ChannelMask;
  byte SubFormat[16];
};

typedef wav::sample data;

struct audio
{
  u32 SampleCount;
  u32 ChannelCount;
  wav::data *SampleData;
};

constexpr inline bool32 IsWaveFormatType(riff::chunk *Chunk)
{
  return Chunk->FormatType == wav::CHUNKID_WAVE;
}

constexpr inline bool32 IsFormatChunk(riff::sub_chunk *SubChunk)
{
  return SubChunk->ChunkID == wav::CHUNKID_FORMAT;
}

constexpr inline bool32 IsDataChunk(riff::sub_chunk *SubChunk)
{
  return SubChunk->ChunkID == wav::CHUNKID_DATA;
}

constexpr inline wav::audio ZeroAudio()
{
  return {
      .SampleCount = 0,
      .SampleData = 0x0,
  };
}

riff::chunk *GetChunk(key Length, void *Data);
wav::format *GetFormat(key Length, void *Data);
wav::audio GetAudio(key Length, void *Data);
} // namespace wav
