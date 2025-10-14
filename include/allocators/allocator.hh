/*
Common code for allocators
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

#define Allocate(_Block, _Type) (_Type *)allocator::_Allocate(_Block, alignof(_Type), sizeof(_Type))
#define AllocateN(_Block, _Type, _Length)                                                          \
  (_Type *)allocator::_Allocate(_Block, alignof(_Type), sizeof(_Type) * _Length)

inline key_diff GetPadding(const void *Offset, const key Align)
{
  // align padding by power of two
  return -(key_diff)Offset & (Align - 1);
}
