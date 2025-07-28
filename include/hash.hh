/*
Hash functions for various purposes.
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

namespace hash
{
// SOURCE: taken from stb_ds
// Copyright (c) 2019 Sean Barrett
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#define __HASH__DEFAULT_SEED 0xD49EE70C
#define __HASH__SIZE_T_BITS ((sizeof(u32)) * 8)
#define __HASH__ROTATE_LEFT(val, n) (((val) << (n)) | ((val) >> (__HASH__SIZE_T_BITS - (n))))
#define __HASH__ROTATE_RIGHT(val, n) (((val) >> (n)) | ((val) << (__HASH__SIZE_T_BITS - (n))))

// Thomas Wang 64 bit to 32 bit
constexpr inline u64 Mix(const u64 Seed, const u32 Value)
{
  u64 Result = Value;
  Result ^= Seed;
  Result = (~Result) + (Result << 18);
  Result ^= Result ^ __HASH__ROTATE_RIGHT(Result, 31);
  Result = Result * 21;
  Result ^= Result ^ __HASH__ROTATE_RIGHT(Result, 11);
  Result += (Result << 6);
  Result ^= __HASH__ROTATE_RIGHT(Result, 22);
  return Result + Seed;
}

constexpr inline u64 Mix(const u32 Value)
{
  return hash::Mix(__HASH__DEFAULT_SEED, Value);
}

inline u64 Mix(const u64 Seed, const char *String)
{
  u64 Result = Seed;
  while (*String)
  {
    Result = __HASH__ROTATE_LEFT(Result, 9) + (unsigned char)*String++;
  }

  return hash::Mix(Result) + Seed;
}

inline u64 Mix(const char *String)
{
  return hash::Mix(__HASH__DEFAULT_SEED, String);
}
/////////// End of stb_ds code ///////////

constexpr inline u64 CantorPair(const u32 X, const u32 Y)
{
  return (X + Y) * (X + Y + 1) / 2 + Y;
}

constexpr inline u64 CantorPairSigned(const i32 X, const i32 Y)
{
  u32 A = X >= 0 ? 2 * X : -2 * X - 1;
  u32 B = Y >= 0 ? 2 * Y : -2 * Y - 1;

  return hash::CantorPair(A, B);
}

constexpr inline u64 SzudzikPair(const u32 X, const u32 Y)
{
  return X >= Y ? (X * X) + X + Y : (Y * Y) + X;
}

constexpr inline u64 SzudzikPairSigned(const i32 X, const i32 Y)
{
  u32 A = X >= 0 ? 2 * X : -2 * X - 1;
  u32 B = Y >= 0 ? 2 * Y : -2 * Y - 1;

  return hash::SzudzikPair(A, B);
}
}; // namespace hash
