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
