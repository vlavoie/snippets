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

  // align padding by power of two
  key_diff Padding = -(key_diff)Allocator->Offset & (Align - 1);
  key_diff Available = Allocator->End - Allocator->Offset - Padding;

  if (Size > Available)
  {
    if (Allocator->Next == 0x0)
    {
      Allocator->Next = CreateBump(Allocator->End - Allocator->Begin);
    }

    return allocator::_Allocate(Allocator->Next, Align, Size);
  }

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
