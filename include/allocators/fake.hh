#pragma once

#include "../common.hh"

namespace allocator
{
struct fake
{
  key Allocated;
};

void *_Allocate(allocator::fake *Allocator, const key Size);
allocator::fake *CreateFake();
void Reset(allocator::fake *Allocator);
void Destroy(allocator::fake *Allocator);
key MemoryUsed(allocator::fake *Allocator);
}; // namespace allocator
