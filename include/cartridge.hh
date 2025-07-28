/*
Header for cartridge asset packer.
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

//
#ifndef __CRPK__Allocate
#include <stdlib.h>
#define __CRPK__Allocate(_N) calloc(1, _N)
#define __CRPK__Free(_Ptr) free(_Ptr)
#endif
//
#ifndef __CRPK__Copy
#include <string.h>
#define __CRPK__Copy memcpy
#endif
//
#ifndef __CRPK__Open
#include <stdio.h>
#define __CRPK__Open fopen
#define __CRPK__Read fread
#define __CRPK__Write fwrite
#define __CRPK__Close fclose
#define __CRPK__Seek fseek
#define __CRPK__Tell ftell
#define __CRPK__file FILE
#define __CRPK__seek_set SEEK_SET
#define __CRPK__seek_end SEEK_END
#endif
//

namespace crpk
{
typedef i32 code;

#define __CRPK__CRPK_EXTENSION_LENGTH 4
#define __CRPK__VERSION 1

#define __CRPK__CODE FourCC('c', 'r', 'p', 'k')

enum return_code
{
  RETURN_CODE_OUTPUT_ERROR = -1,
  RETURN_CODE_SUCCESS = 0,
  RETURN_CODE_INPUT_FILE_ERROR = 1, // Error code 1..N is error at input file at Index + 1
};

struct header
{
  u32 Extension;
  u32 Padding;
  u64 Version;
  u64 BlockCount;
  u64 DataSize;
};

struct block
{
  u64 ID;
  u64 Hash;
  u64 StartOffset;
  u64 Length;
};
//

struct cartridge
{
  crpk::header Header;
  crpk::block *Blocks;
  byte *Data;
};

struct buffer
{
  key Length;
  byte *Data;
};

crpk::buffer GetKeyData(crpk::cartridge *Cartridge, const char *AssetFile);

crpk::code Package(key Length, const char **InputFiles, const char *Output);
crpk::cartridge *Unpack(const char *CartridgeFile);
crpk::cartridge *Unpack(void *CartridgeData);
}; // namespace crpk
