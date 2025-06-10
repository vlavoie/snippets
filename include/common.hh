#pragma once

// libc
#include <stddef.h>
#include <stdint.h>
//

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

#define InvalidCodePath *(volatile i32 *)0 = 0

#define FourCC(A, B, C, D) ((u32(D) << 24) | (u32(C) << 16) | (u32(B) << 8) | (u32(A)))

#define ArrayLength(_Array) (sizeof(_Array) / sizeof(*_Array))
