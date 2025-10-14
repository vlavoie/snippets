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
