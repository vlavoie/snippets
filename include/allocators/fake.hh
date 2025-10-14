/*
Fake allocator header
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

#include "allocator.hh"
#include "../common.hh"

namespace allocator
{
struct fake
{
  byte *Begin;
  byte *Offset;
};

void *_Allocate(allocator::fake *Allocator, const key Align, const key Size);
allocator::fake *CreateFake();
void Reset(allocator::fake *Allocator);
void Destroy(allocator::fake *Allocator);
key MemoryUsed(allocator::fake *Allocator);
}; // namespace allocator
