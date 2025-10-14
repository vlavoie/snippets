/*
Bump allocator header
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
struct bump
{
  byte *Begin;
  byte *Offset;
  byte *End;
};

void *_Allocate(allocator::bump *Allocator, const key Align, const key Size);
allocator::bump *CreateBump(const key Size);
void Reset(allocator::bump *Allocator);
void Destroy(allocator::bump *Allocator);
key MemoryUsed(allocator::bump *Allocator);
}; // namespace allocator
