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
