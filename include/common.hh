/*
Common header for types and other re-used inline functions and macros.
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

// libc
#include <stddef.h>
#include <stdint.h>

#ifndef SysAllocate
#include <stdlib.h>
#define SysAllocate(_Type, _N) (_Type *)calloc(_N, sizeof(_Type))
#define SysFree(_Ptr) free(_Ptr)
#endif
//

#define BYTE 1
#define KILOBYTE (BYTE * 1024)
#define MEGABYTE (KILOBYTE * 1024)
#define GIGABYTE (MEGABYTE * 1024)

#define Allocate(_Block, _Type) (_Type *)allocator::_Allocate(_Block, alignof(_Type), sizeof(_Type))
#define AllocateN(_Block, _Type, _Length)                                                          \
  (_Type *)allocator::_Allocate(_Block, alignof(_Type), sizeof(_Type) * _Length)

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u8 bool8;
typedef u32 bool32;

typedef float f32;
typedef double f64;

typedef u8 byte;
typedef u16 word;

typedef size_t key;
typedef ptrdiff_t key_diff;

typedef u64 tick;

#define I8_MAX INT8_MAX
#define I16_MAX INT16_MAX
#define I32_MAX INT32_MAX
#define I64_MAX INT64_MAX

#define U8_MAX UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX

#define BYTE_MAX U8_MAX
#define KEY_MAX SIZE_MAX

#define Assert(Condition, Message)                                                                 \
  if (!(Condition && Message))                                                                     \
  {                                                                                                \
    *(volatile i32 *)0 = 0;                                                                        \
  }

#define FourCC(A, B, C, D) ((u32(D) << 24) | (u32(C) << 16) | (u32(B) << 8) | (u32(A)))

#define ArrayLength(_Array) (sizeof(_Array) / sizeof(*_Array))

typedef u32 color;

struct rgb
{
  f32 R, G, B;
};

struct rgba
{
  f32 R, G, B, A;
};

constexpr inline u32 PackColor(f32 R, f32 G, f32 B, f32 A)
{
  return u32((u8(R * f32(U8_MAX)) << 24) | (u8(G * f32(U8_MAX)) << 16) |
             (u8(B * f32(U8_MAX)) << 8) | (u8(A * f32(U8_MAX))));
};

constexpr inline u32 PackRGBA(const rgba RGBA)
{
  return PackColor(RGBA.R, RGBA.G, RGBA.B, RGBA.A);
}

constexpr inline u32 PackRGB(const rgb RGB)
{
  return PackColor(RGB.R, RGB.G, RGB.B, 1.0f);
}

constexpr inline f32 UnpackRed(const u32 Color)
{
  return f32((Color & 0xFF000000) >> 24) / f32(U8_MAX);
}

constexpr inline f32 UnpackGreen(const u32 Color)
{
  return f32((Color & 0x00FF0000) >> 16) / f32(U8_MAX);
}

constexpr inline f32 UnpackBlue(const u32 Color)
{
  return f32((Color & 0x0000FF00) >> 8) / f32(U8_MAX);
}

constexpr inline f32 UnpackAlpha(const u32 Color)
{
  return f32(Color & 0x000000FF) / f32(U8_MAX);
}

constexpr inline rgb UnpackRGB(const color Color)
{
  return {
      .R = UnpackRed(Color),
      .G = UnpackGreen(Color),
      .B = UnpackBlue(Color),
  };
}

constexpr inline rgba UnpackRGBA(const color Color)
{
  return {
      .R = UnpackRed(Color),
      .G = UnpackGreen(Color),
      .B = UnpackBlue(Color),
      .A = UnpackAlpha(Color),
  };
}

inline void SetFlag(key *Field, const key Flag)
{
  *Field |= Flag;
}

inline bool32 HasFlag(key *Field, const key Flag)
{
  return *Field & Flag;
}

inline void RemoveFlag(key *Field, const key Flag)
{
  *Field &= ~Flag;
}
