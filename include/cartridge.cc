/*
Implementation for cartridge asset packer.
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

#include "cartridge.hh"
#include "hash.hh"

#define __CRPK__BUFFER_SIZE 2048
#define __CRPK__Offset(_Ptr, _N) ((byte *)_Ptr + _N)
#define __CRPK__SEED_ID 0x014F65CB
#define __CRPK__SEED_HASH 0xD49EE70C

u64 IDString(const char *AssetFile)
{
  return hash::Mix(__CRPK__SEED_ID, AssetFile);
}

u64 HashString(const char *AssetFile)
{
  return hash::Mix(__CRPK__SEED_HASH, AssetFile);
}

crpk::block *GetBlockWithID(crpk::block *Blocks, key Length, u64 Hash, u64 ID)
{
  key Index = Hash % Length;
  crpk::block *Block = Blocks + Index;
  key Count = 0;

  while (Block->ID != ID)
  {
    Index = (Index + 1) % Length;
    Block = Blocks + Index;
    Count++;

    // prevent infinite loop in case of full iteration
    if (Count >= Length)
    {
      return 0x0;
    }
  }

  return Block;
}

crpk::block *GetEmptyHashIndex(crpk::block *Blocks, key Length, u64 Hash)
{
  return GetBlockWithID(Blocks, Length, Hash, 0);
}

key CartridgeSizeof(crpk::header *Header)
{
  return sizeof(crpk::cartridge) + sizeof(crpk::block) * Header->BlockCount +
         sizeof(byte) * Header->DataSize;
}

crpk::cartridge *crpk::Unpack(const char *CartridgeFile)
{
  __CRPK__file *File = __CRPK__Open(CartridgeFile, "rb");

  if (!File)
  {
    return 0x0;
  }

  crpk::header Header;
  __CRPK__Read(&Header, sizeof(crpk::header), 1, File);

  if (Header.Extension != __CRPK__CODE || Header.Version != __CRPK__VERSION)
  {
    return 0x0;
  }

  void *CartridgeMemory = __CRPK__Allocate(CartridgeSizeof(&Header));

  if (!CartridgeMemory)
  {
    return 0x0;
  }

  crpk::cartridge *Cartridge = (crpk::cartridge *)CartridgeMemory;
  Cartridge->Header = Header;

  key Offset = sizeof(crpk::cartridge);
  Cartridge->Blocks = (crpk::block *)__CRPK__Offset(CartridgeMemory, Offset);
  __CRPK__Read(Cartridge->Blocks, sizeof(crpk::block), Cartridge->Header.BlockCount, File);

  Offset += sizeof(crpk::block) * Cartridge->Header.BlockCount;
  Cartridge->Data = (byte *)__CRPK__Offset(CartridgeMemory, Offset);
  __CRPK__Read(Cartridge->Data, sizeof(byte), Cartridge->Header.DataSize, File);

  return Cartridge;
}

crpk::cartridge *crpk::Unpack(void *CartridgeData)
{
  if (!CartridgeData)
  {
    return 0x0;
  }

  crpk::header *Header = (crpk::header *)CartridgeData;

  if (Header->Extension != __CRPK__CODE || Header->Version != __CRPK__VERSION)
  {
    return 0x0;
  }

  void *CartridgeMemory = __CRPK__Allocate(CartridgeSizeof(Header));

  if (!CartridgeMemory)
  {
    return 0x0;
  }

  crpk::cartridge *Cartridge = (crpk::cartridge *)CartridgeMemory;
  Cartridge->Header = *Header;

  key Offset = sizeof(crpk::cartridge);
  Cartridge->Blocks = (crpk::block *)__CRPK__Offset(CartridgeMemory, Offset);
  __CRPK__Copy(Cartridge->Blocks, __CRPK__Offset(CartridgeData, Offset),
               Cartridge->Header.BlockCount);

  Offset += sizeof(crpk::block) * Cartridge->Header.BlockCount;
  Cartridge->Data = (byte *)__CRPK__Offset(CartridgeMemory, Offset);
  __CRPK__Copy(Cartridge->Data, __CRPK__Offset(CartridgeData, Offset), Cartridge->Header.DataSize);

  return Cartridge;
}

crpk::buffer crpk::GetKeyData(crpk::cartridge *Cartridge, const char *AssetFile)
{
  u64 Hash = HashString(AssetFile), ID = IDString(AssetFile);
  crpk::block *Block = GetBlockWithID(Cartridge->Blocks, Cartridge->Header.BlockCount, Hash, ID);

  if (Block)
  {
    return {
        .Length = key(Block->Length),
        .Data = Cartridge->Data + Block->StartOffset,
    };
  }

  return {
      .Length = 0,
      .Data = 0x0,
  };
}

crpk::code crpk::Package(key Length, const char **InputFiles, const char *Output)
{
  crpk::header Header = {
      .Extension = __CRPK__CODE,
      .Padding = 0,
      .Version = __CRPK__VERSION,
      .BlockCount = Length,
      .DataSize = 0,
  };

  key FileLength = 0;
  key Offset = 0;
  crpk::block *Blocks = (crpk::block *)__CRPK__Allocate(sizeof(crpk::block) * Length);

  for (key Index = 0; Index < Length; Index++)
  {
    const char *InputFile = InputFiles[Index];
    u64 ID = IDString(InputFile), Hash = HashString(InputFile);
    crpk::block *Block = GetEmptyHashIndex(Blocks, Length, Hash);

    if (!Block)
    {
      return Index + 1;
    }

    __CRPK__file *Asset = __CRPK__Open(InputFile, "rb");

    if (!Asset)
    {
      return Index + 1;
    }

    if (__CRPK__Seek(Asset, 0, __CRPK__seek_end))
    {
      __CRPK__Close(Asset);
      return Index + 1;
    }

    FileLength = __CRPK__Tell(Asset);
    __CRPK__Close(Asset);

    Block->ID = ID;
    Block->Hash = Hash;
    Block->Length = FileLength;
    Block->StartOffset = Offset;

    Offset += FileLength;
  }

  Header.DataSize = Offset;

  __CRPK__file *CartridgeOutput = __CRPK__Open(Output, "wb");

  if (!CartridgeOutput)
  {
    return crpk::RETURN_CODE_OUTPUT_ERROR;
  }

  __CRPK__Write(&Header, sizeof(Header), 1, CartridgeOutput);
  __CRPK__Write(Blocks, sizeof(crpk::block), Length, CartridgeOutput);
  __CRPK__Free(Blocks);

  byte Buffer[__CRPK__BUFFER_SIZE];
  key ReadLength = 0;

  for (key Index = 0; Index < Length; Index++)
  {
    const char *InputFile = InputFiles[Index];
    __CRPK__file *Asset = __CRPK__Open(InputFile, "rb");

    if (!Asset)
    {
      __CRPK__Close(CartridgeOutput);
      return Index + 1;
    }

    if (__CRPK__Seek(Asset, 0, __CRPK__seek_set))
    {
      __CRPK__Close(Asset);
      __CRPK__Close(CartridgeOutput);
      return Index + 1;
    }

    do
    {
      ReadLength = __CRPK__Read(Buffer, sizeof(byte), __CRPK__BUFFER_SIZE, Asset);
      __CRPK__Write(Buffer, sizeof(byte), ReadLength, CartridgeOutput);
    } while (ReadLength == __CRPK__BUFFER_SIZE);

    __CRPK__Close(Asset);
  }
  __CRPK__Close(CartridgeOutput);

  return crpk::RETURN_CODE_SUCCESS;
}
