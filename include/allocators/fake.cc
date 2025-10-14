#include "fake.hh"

void *allocator::_Allocate(allocator::fake *Allocator, const key Size)
{
  Allocator->Allocated += Size;
  return 0x0;
}

allocator::fake *CreateFake()
{
  return SysAllocate(allocator::fake, 1);
}

void allocatorReset(allocator::fake *Allocator)
{
  Allocator->Allocated = 0;
}

void allocatorDestroy(allocator::fake *Allocator)
{
  SysFree(Allocator);
}

key allocatorMemoryUsed(allocator::fake *Allocator)
{
  return Allocator->Allocated;
}
