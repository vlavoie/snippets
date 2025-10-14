/*
Fake allocator implementation
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

#include "fake.hh"

void *allocator::_Allocate(allocator::fake *Allocator, const key Align, const key Size)
{
  key_diff Padding = GetPadding(Allocator->Offset, Align);
  Allocator->Offset += Size + Padding;
  return 0x0;
}

allocator::fake *allocator::CreateFake()
{
  allocator::fake *Allocator = SysAllocate(allocator::fake, 1);
  Allocator->Begin = (byte *)Allocator;
  Allocator->Offset = Allocator->Begin;

  return Allocator;
}

void allocator::Reset(allocator::fake *Allocator)
{
  Allocator->Offset = Allocator->Begin;
}

void allocator::Destroy(allocator::fake *Allocator)
{
  SysFree(Allocator);
}

key allocator::MemoryUsed(allocator::fake *Allocator)
{
  return Allocator->Offset - Allocator->Begin;
}
