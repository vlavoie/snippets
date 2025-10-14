#pragma once

#include <common.hh>

#define Allocate(_Block, _Type) (_Type *)allocator::_Allocate(_Block, alignof(_Type), sizeof(_Type))
#define AllocateN(_Block, _Type, _Length)                                                          \
  (_Type *)allocator::_Allocate(_Block, alignof(_Type), sizeof(_Type) * _Length)

inline key_diff GetPadding(const void *Offset, const key Align)
{
  // align padding by power of two
  return -(key_diff)Offset & (Align - 1);
}
