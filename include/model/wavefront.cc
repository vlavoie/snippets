#include "wavefront.hh"

obj::mesh *obj::Parse(key Length, void *Data, i32 *ErrorCode)
{
  return 0x0;
}

obj::mesh *obj::Parse(key Length, void *Data)
{
  i32 ErrorCode;
  return obj::Parse(Length, Data, &ErrorCode);
}
