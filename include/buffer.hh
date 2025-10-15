#pragma once

#include "common.hh"

#include <stdio.h>
#include <string.h>

struct buffer
{
  key Length;
  void *Data;
};

struct buffer_ptr
{
  buffer Buffer;
  key Offset;
};

constexpr inline buffer ZeroLengthBuffer()
{
  return {.Length = 0, .Data = 0x0};
}

inline bool32 IsInitialized(const buffer Buffer)
{
  return Buffer.Length && Buffer.Data;
}

inline buffer AllocateBuffer(const key Length)
{
  Assert(Length > 0, "Request data allocation of 0 bytes.");

  return {
      .Length = Length,
      .Data = SysAllocate(byte, Length),
  };
}

inline void FreeBuffer(const buffer Buffer)
{
  Assert(IsInitialized(Buffer), "Buffer was already freed or not initialized.");

  SysFree(Buffer.Data);
}

inline buffer_ptr BufferPointer(const buffer Buffer)
{
  return {
      .Buffer = Buffer,
      .Offset = 0,
  };
}

inline buffer LoadBufferFromFile(const char *Path)
{
  FILE *File = fopen(Path, "r");

  if (File)
  {
    buffer Result;

    fseek(File, 0, SEEK_END);
    Result.Length = ftell(File);
    Result.Data = SysAllocate(byte, Result.Length);
    rewind(File);
    fread(Result.Data, 1, Result.Length, File);

    fclose(File);

    return Result;
  }
  else
  {
    return ZeroLengthBuffer();
  }
}

inline bool32 WriteFileFromBuffer(const buffer Buffer, const char *Path)
{
  FILE *File = fopen(Path, "w");

  if (File)
  {
    fwrite(Buffer.Data, 1, Buffer.Length, File);
    fclose(File);

    return true;
  }

  return false;
}

inline void *_ReadBuffer(buffer_ptr *Ptr, key N)
{
  Assert(Ptr->Buffer.Length >= Ptr->Offset + N, "Buffer reading went out of bounds.");

  void *Result = ((byte *)Ptr->Buffer.Data) + Ptr->Offset;
  Ptr->Offset += N;

  return Result;
}

#define ReadBuffer(_Ptr, _Type, _N) (_Type *)_ReadBuffer(_Ptr, sizeof(_Type) * _N)

inline void _WriteBuffer(buffer_ptr *Ptr, void *Source, key N)
{
  Assert(Ptr->Buffer.Length >= Ptr->Offset + N, "Buffer writing went out of bounds.");
  void *Destination = ((byte *)Ptr->Buffer.Data) + Ptr->Offset;
  memcpy(Destination, Source, N);
  Ptr->Offset += N;
}

#define WriteBuffer(_Ptr, _Source, _Type, _N) _WriteBuffer(_Ptr, _Source, sizeof(_Type) * _N)
