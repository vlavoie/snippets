/*
Bump allocator implementation
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

#include "bump.hh"

allocator::bump *allocator::CreateBump(const key Size)
{
  key HeaderSize = sizeof(allocator::bump);
  byte *RawMemory = SysAllocate(byte, Size + HeaderSize);
  allocator::bump *Output = (allocator::bump *)RawMemory;

  Output->Begin = RawMemory + HeaderSize;
  Output->Offset = Output->Begin;
  Output->End = Output->Begin ? Output->Begin + Size : 0x0;

  return Output;
}

void *allocator::_Allocate(allocator::bump *Allocator, const key Align, const key Size)
{
  if (Size == 0)
  {
    return 0x0;
  }

  key_diff Padding = GetPadding(Allocator->Offset, Align);
  key_diff Available = Allocator->End - Allocator->Offset - Padding;

  Assert(Available > 0 && Size <= Available, "Allocator overflow.");

  void *Output = Allocator->Offset + Padding;
  Allocator->Offset += Size + Padding;
  return Output;
}

void allocator::Reset(allocator::bump *Allocator)
{
  Allocator->Offset = Allocator->Begin;
}

void allocator::Destroy(allocator::bump *Allocator)
{
  SysFree(Allocator);
}

key allocator::MemoryUsed(allocator::bump *Allocator)
{
  return Allocator->Offset - Allocator->Begin;
}
