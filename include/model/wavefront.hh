#include <common.hh>

// libc
#ifndef __OBJ__Read
#include <unistd.h>
#define __OBJ__Read pread
#endif

#ifndef __OBJ__Allocate
#include <stdlib.h>
#define __OBJ__Allocate(_N) malloc(_N)
#endif

namespace obj
{
struct vertex
{
  f32 X, Y, Z, W;
};

struct texture_coord
{
  f32 U, V, W;
};

struct vertex_normal
{
  f32 X, Y, Z;
};

struct face_vertex
{
  key VertexIndex;
  key TextureCoordIndex;
  key NormalIndex;
};

// Only support triangles for now, variable length faces introduce overhead
struct triangle_face
{
  face_vertex Faces[3];
};

// Don't support this for now
// struct space_vertex
// {
//   f32 U, V, W;
// };

// struct line
// {
//   key VertexIndex;
// };
////

struct mesh
{
  key VertexCount, TextureCoordCount, NormalCount, TriangleCount;
  obj::vertex *Vertices;
  obj::texture_coord *TextureCoords;
  obj::vertex_normal *Normals;
  obj::triangle_face *Triangles;
};

obj::mesh *Parse(key Length, void *Data);
obj::mesh *Parse(key Length, void *Data, i32 *ErrorCode);
} // namespace obj
