/*
Header for 2D mathematics.
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

#include "common.hh"

// math.h
#ifndef __MATH2D__Sinf
#include <math.h>
#define __MATH2D__Sinf sinf
#define __MATH2D__Cosf cosf
#define __MATH2D__Powf powf
#define __MATH2D__Maxf fmax
#define __MATH2D__Minf fmin
#define __MATH2D__Sqrt sqrt
#define __MATH2D__Abs abs
#define __MATH2D__Floor floor
#define __MATH2D__Ceil ceil
#define __MATH2D__Roundf roundf
#define __MATH2D__Atan2 atan2f
#define __MATH2D__Sign copysignf
#endif
//

#define MATH2D_EPSILON f32(2.2204460492503130808472633361816E-16f)
#define MATH2D_PI f32(3.14159265f)
#define MATH2D_PI_TWO f32(6.28318531f)

#define MATH2D_CLOCKWISE f32(1.0f)
#define MATH2D_COUNTERCLOCKWISE f32(-1.0f)

#define ZeroVector2 vec2{0.0f, 0.0f}

struct vec2
{
  f32 X, Y;
};

struct matrix2
{
  f32 X0, Y0, X1, Y1;
};

struct box
{
  vec2 AA, BB;
};

struct circle
{
  vec2 Position;
  f32 Radius;
};

struct ray
{
  vec2 Origin, Direction, InverseDirection;
};

struct line
{
  vec2 Start, End;
};

constexpr inline f32 Sin(const f32 Input)
{
  return __MATH2D__Sinf(Input);
}

constexpr inline f32 Cos(const f32 Input)
{
  return __MATH2D__Cosf(Input);
}

constexpr inline f32 Pow(const f32 Input, const f32 Exponent)
{
  return __MATH2D__Powf(Input, Exponent);
}

constexpr inline f32 Maximum(const f32 L, const f32 R)
{
  return __MATH2D__Maxf(L, R);
}

constexpr inline f32 Minimum(const f32 L, const f32 R)
{
  return __MATH2D__Minf(L, R);
}

constexpr inline f32 SquareRoot(const f32 Value)
{
  return __MATH2D__Sqrt(Value);
}

constexpr inline f32 Absolute(const f32 Value)
{
  return __MATH2D__Abs(Value);
}

constexpr inline f32 Floor(const f32 Value)
{
  return __MATH2D__Floor(Value);
}

constexpr inline f32 Ceil(const f32 Value)
{
  return __MATH2D__Ceil(Value);
}

constexpr inline f32 Round(const f32 Value)
{
  return __MATH2D__Roundf(Value);
}

constexpr inline f32 Atan2(const f32 Y, const f32 X)
{
  return __MATH2D__Atan2(Y, X);
}

constexpr inline i32 Sign(const f32 Value)
{
  return __MATH2D__Sign(1.0f, Value);
}

constexpr inline f32 Square(const f32 V)
{
  return V * V;
}

constexpr inline f32 Lerp(const f32 A, const f32 B, const f32 Delta)
{
  return A * (1.0f - Delta) + B * Delta;
}

constexpr inline f32 LerpAngle(const f32 A, const f32 B, const f32 Theta)
{
  f32 Correction = A, Difference = B - A;

  if (Difference > MATH2D_PI)
  {
    Correction += MATH2D_PI_TWO;
  }
  else if (Difference < -MATH2D_PI)
  {
    Correction -= MATH2D_PI_TWO;
  }

  return Lerp(Correction, B, Theta);
}

constexpr inline f32 LerpAngle(const f32 A, const f32 B, const f32 Theta, const f32 Direction)
{
  f32 Correction = B;

  if (Direction < 0.0f) // COUNTERCLOCKWISE
  {
    if (A < B)
    {
      Correction -= MATH2D_PI_TWO;
    }
  }
  else
  {
    if (A > B)
    {
      Correction += MATH2D_PI_TWO;
    }
  }

  return Lerp(A, Correction, Theta);
}

constexpr inline f32 Rotate(const f32 A, const f32 B, const f32 Theta)
{
  f32 Difference = B - A;
  f32 Correction = A;

  if (Difference > MATH2D_PI)
  {
    Correction += MATH2D_PI_TWO;
    Difference = B - Correction;
  }
  else if (Difference < -MATH2D_PI)
  {
    Correction -= MATH2D_PI_TWO;
    Difference = B - Correction;
  }

  f32 Rotation = Theta * Sign(Difference);

  if (Absolute(Difference) < Theta)
  {
    return Correction + Difference;
  }

  return Correction + Rotation;
}

constexpr inline vec2 operator+(const vec2 L, const vec2 R)
{
  return {
      .X = L.X + R.X,
      .Y = L.Y + R.Y,
  };
}

constexpr inline vec2 operator+(const vec2 L, const f32 V)
{
  return {
      .X = L.X + V,
      .Y = L.Y + V,
  };
}

constexpr inline vec2 operator+(const f32 V, const vec2 R)
{
  return R + V;
}

constexpr inline vec2 &operator+=(vec2 &L, const f32 V)
{
  L = L + V;
  return L;
}

constexpr inline vec2 &operator+=(vec2 &L, const vec2 R)
{
  L = L + R;
  return L;
}

constexpr inline vec2 operator-(const vec2 L, const vec2 R)
{
  return {
      .X = L.X - R.X,
      .Y = L.Y - R.Y,
  };
}

constexpr inline vec2 operator-(const vec2 L, const f32 V)
{
  return {
      .X = L.X - V,
      .Y = L.Y - V,
  };
}

constexpr inline vec2 operator-(const f32 V, const vec2 R)
{
  return {
      .X = V - R.X,
      .Y = V - R.Y,
  };
}

constexpr inline vec2 operator-=(vec2 &L, const f32 V)
{
  L = L - V;
  return L;
}

constexpr inline vec2 &operator-=(vec2 &L, const vec2 R)
{
  L = L - R;
  return L;
}

constexpr inline vec2 operator*(const vec2 L, const f32 V)
{
  return {
      .X = L.X * V,
      .Y = L.Y * V,
  };
}

constexpr inline vec2 operator*(const f32 V, const vec2 R)
{
  return R * V;
}

constexpr inline vec2 operator*(const vec2 L, const vec2 R)
{
  return {
      .X = L.X * R.X,
      .Y = L.Y * R.Y,
  };
}

constexpr inline vec2 &operator*=(vec2 &L, const f32 V)
{
  L = L * V;
  return L;
}

constexpr inline vec2 &operator*=(vec2 &L, const vec2 R)
{
  L = L * R;
  return L;
}

constexpr inline vec2 operator/(const vec2 L, const f32 V)
{
  return {
      .X = L.X / V,
      .Y = L.Y / V,
  };
}

constexpr inline vec2 operator/(const f32 V, const vec2 R)
{
  return {
      .X = V / R.X,
      .Y = V / R.Y,
  };
}

constexpr inline vec2 operator/(const vec2 L, const vec2 R)
{
  return {
      .X = L.X / R.X,
      .Y = L.Y / R.Y,
  };
}

constexpr inline vec2 &operator/=(vec2 &L, const f32 V)
{
  L = L / V;
  return L;
}

constexpr inline vec2 &operator/=(vec2 &L, const vec2 R)
{
  L = L / R;
  return L;
}

constexpr inline vec2 operator*(const vec2 V, const matrix2 M)
{
  return {
      .X = M.X0 * V.X + M.Y0 * V.Y,
      .Y = M.X1 * V.X + M.Y1 * V.Y,
  };
}

constexpr inline vec2 operator*(const matrix2 M, const vec2 V)
{
  return V * M;
}

constexpr inline vec2 &operator*=(vec2 &L, const matrix2 M)
{
  L = L * M;
  return L;
}

constexpr inline vec2 Rotate(const vec2 V, const f32 Theta)
{
  matrix2 M = {
      .X0 = Cos(Theta),
      .Y0 = Sin(Theta),
      .X1 = -Sin(Theta),
      .Y1 = Cos(Theta),
  };

  return V * M;
}

constexpr inline f32 FastLength(const vec2 Value)
{
  return Square(Value.X) + Square(Value.Y);
}

constexpr inline bool32 operator<(const vec2 L, const vec2 R)
{
  return FastLength(L) < FastLength(R);
}

constexpr inline bool32 operator>(const vec2 L, const vec2 R)
{
  return FastLength(L) > FastLength(R);
}

constexpr inline bool32 operator<=(const vec2 L, const vec2 R)
{
  return FastLength(L) <= FastLength(R);
}

constexpr inline bool32 operator>=(const vec2 L, const vec2 R)
{
  return FastLength(L) >= FastLength(R);
}

constexpr inline vec2 Floor(const vec2 Value)
{
  return {
      .X = __MATH2D__Floor(Value.X),
      .Y = __MATH2D__Floor(Value.Y),
  };
}

constexpr inline vec2 Absolute(const vec2 Value)
{
  return vec2{__MATH2D__Abs(Value.X), __MATH2D__Abs(Value.Y)};
}

constexpr inline f32 Length(const vec2 V)
{
  return SquareRoot(Square(V.X) + Square(V.Y));
}

inline f32 Distance(const vec2 L, const vec2 R)
{
  vec2 D = R - L;
  return SquareRoot(Square(D.X) + Square(D.Y));
}

constexpr inline bool32 IsZeroLength(const vec2 V)
{
  return FastLength(V) < MATH2D_EPSILON;
}

constexpr inline bool32 IsNotZeroLength(const vec2 V)
{
  return FastLength(V) > 0.0f;
}

constexpr inline vec2 Normalize(const vec2 V)
{
  return IsNotZeroLength(V) ? V / Length(V) : ZeroVector2;
}

constexpr inline f32 CompositionMinimum(const vec2 V)
{
  return Minimum(V.X, V.Y);
}

constexpr inline f32 CompositionMaximum(const vec2 V)
{
  return Maximum(V.X, V.Y);
}

constexpr inline vec2 Minimum(const vec2 L, const vec2 R)
{
  return L < R ? L : R;
}

constexpr inline vec2 Maximum(const vec2 L, const vec2 R)
{
  return L > R ? L : R;
}

constexpr inline vec2 MixMinimum(const vec2 L, const vec2 R)
{
  return {
      .X = Minimum(L.X, R.X),
      .Y = Minimum(L.Y, R.Y),
  };
}

constexpr inline vec2 MixMaximum(const vec2 L, const vec2 R)
{
  return {
      .X = Maximum(L.X, R.X),
      .Y = Maximum(L.Y, R.Y),
  };
}

constexpr inline vec2 AngleVector(const f32 Angle)
{
  return {
      .X = Cos(Angle),
      .Y = Sin(Angle),
  };
}

constexpr inline f32 VectorAngle(const vec2 Vector)
{
  f32 Angle = Atan2(Vector.Y, Vector.X);
  return Angle > 0.0f ? Angle : MATH2D_PI_TWO + Angle;
}

constexpr inline f32 Dot(const vec2 L, const vec2 R)
{
  return L.X * R.X + L.Y * R.Y;
}

constexpr inline box operator+(const box L, const vec2 V)
{
  return {
      .AA = L.AA + V,
      .BB = L.BB + V,
  };
}

constexpr inline box &operator+=(box &L, const vec2 V)
{
  L = L + V;
  return L;
}

constexpr inline box operator-(const box L, const vec2 V)
{
  return {
      .AA = L.AA - V,
      .BB = L.BB - V,
  };
}

constexpr inline box operator-=(box &L, const vec2 V)
{
  L = L - V;
  return L;
}

constexpr inline box FixCoordinates(const box Box)
{
  return {
      .AA = MixMinimum(Box.AA, Box.BB),
      .BB = MixMaximum(Box.AA, Box.BB),
  };
}

constexpr inline line FixCoordinates(const line Line)
{
  return {
      .Start = MixMinimum(Line.Start, Line.End),
      .End = MixMaximum(Line.Start, Line.End),
  };
}

constexpr inline bool32 Intersects(const vec2 Point, const box Box)
{
  return Point.X >= Box.AA.X && Point.X <= Box.BB.X && Point.Y >= Box.AA.Y && Point.Y <= Box.BB.Y;
}

constexpr inline bool32 Intersects(const box Box, const vec2 Point)
{
  return Intersects(Point, Box);
}

constexpr inline bool32 Intersects(const box Left, const box Right)
{
  return Left.AA.X <= Right.BB.X && Left.BB.X >= Right.AA.X && Left.AA.Y <= Right.BB.Y &&
         Left.BB.Y >= Right.AA.Y;
}

constexpr inline f32 DistanceX(const box Left, const box Right)
{
  return Absolute(Maximum(Right.AA.X - Left.BB.X, Left.AA.X - Right.BB.X));
}

constexpr inline f32 DistanceY(const box Left, const box Right)
{
  return Absolute(Maximum(Right.AA.Y - Left.BB.Y, Left.AA.Y - Right.BB.Y));
}

constexpr inline f32 DistanceX(const vec2 Point, const box Box)
{
  return Absolute(Maximum(Box.AA.X - Point.X, Point.X - Box.BB.X));
}

constexpr inline f32 DistanceY(const vec2 Point, const box Box)
{
  return Absolute(Maximum(Box.AA.Y - Point.Y, Point.Y - Box.BB.Y));
}

constexpr inline f32 DistanceX(const box Box, const vec2 Point)
{
  return DistanceX(Point, Box);
}

constexpr inline f32 DistanceY(const box Box, const vec2 Point)
{
  return DistanceY(Point, Box);
}

constexpr inline f32 OverlapX(const box Left, const box Right)
{
  return Maximum(0, Minimum(Left.BB.X, Right.BB.X) - Maximum(Left.AA.X, Right.AA.X));
}

constexpr inline f32 OverlapY(const box Left, const box Right)
{
  return Maximum(0, Minimum(Left.BB.Y, Right.BB.Y) - Maximum(Left.AA.Y, Right.AA.Y));
}

constexpr inline f32 Width(const box Box)
{
  return Box.BB.X - Box.AA.X;
}

constexpr inline f32 Height(const box Box)
{
  return Box.BB.Y - Box.AA.Y;
}

constexpr inline vec2 Dimensions(const box Box)
{
  return Box.BB - Box.AA;
}

constexpr inline bool32 Intersects(const vec2 Point, const circle Circle)
{
  return Distance(Point, Circle.Position) < Circle.Radius;
}

constexpr inline bool32 Intersects(const circle Circle, const vec2 Point)
{
  return Intersects(Point, Circle);
}

constexpr inline bool32 Intersects(const line L, const line R)
{
  f32 DeltaA = ((R.End.X - R.Start.X) * (L.Start.Y - R.Start.Y) -
                (R.End.Y - R.Start.Y) * (L.Start.X - R.Start.X)) /
               ((R.End.Y - R.Start.Y) * (L.End.X - L.Start.X) -
                (R.End.X - R.Start.X) * (L.End.Y - L.Start.Y));
  f32 DeltaB = ((L.End.X - L.Start.X) * (L.Start.Y - R.Start.Y) -
                (L.End.Y - L.Start.Y) * (L.Start.X - R.Start.X)) /
               ((R.End.Y - R.Start.Y) * (L.End.X - L.Start.X) -
                (R.End.X - R.Start.X) * (L.End.Y - L.Start.Y));
  return DeltaA >= 0.0f && DeltaA <= 1.0f && DeltaB >= 0.0f && DeltaB <= 1.0f;
}

constexpr inline bool32 Intersects(const line Line, const box Box)
{
  line Left{Box.AA, vec2{Box.AA.X, Box.BB.Y}};
  line Top{Box.AA, vec2{Box.BB.X, Box.AA.Y}};
  line Right{vec2{Box.BB.X, Box.AA.Y}, Box.BB};
  line Bottom{vec2{Box.AA.X, Box.BB.Y}, Box.BB};

  return Intersects(Line, Left) || Intersects(Line, Top) || Intersects(Line, Right) ||
         Intersects(Line, Bottom);
}

constexpr inline bool32 Intersects(const box Box, const line Line)
{
  return Intersects(Line, Box);
}

constexpr inline box CreateBox(vec2 Origin, vec2 Dimensions)
{
  return {
      .AA = Origin,
      .BB = Origin + Dimensions,
  };
}

constexpr inline ray CreateRay(vec2 Origin, vec2 Direction)
{
  return {
      .Origin = Origin,
      .Direction = Direction,
      .InverseDirection = vec2{1.0f / Direction.X, 1.0f / Direction.Y},
  };
}

constexpr inline bool32 Intersects(box Box, ray Ray)
{
  f32 TX0 = (Box.AA.X - Ray.Origin.X) * Ray.InverseDirection.X;
  f32 TX1 = (Box.BB.X - Ray.Origin.X) * Ray.InverseDirection.X;

  f32 TMin = Minimum(TX0, TX1);
  f32 TMax = Maximum(TX0, TX1);

  f32 TY0 = (Box.AA.Y - Ray.Origin.Y) * Ray.InverseDirection.Y;
  f32 TY1 = (Box.BB.Y - Ray.Origin.Y) * Ray.InverseDirection.Y;

  TMin = Maximum(TMin, Minimum(TY0, TY1));
  TMax = Minimum(TMax, Maximum(TY0, TY1));

  return TMax >= TMin && TMax >= 0.0f;
}

constexpr inline bool32 Intersects(ray Ray, box Box)
{
  return Intersects(Box, Ray);
}

struct matrix4
{
  f32 X0, Y0, Z0, W0, X1, Y1, Z1, W1, X2, Y2, Z2, W2, X3, Y3, Z3, W3;
};

inline void OrthographicMatrix(matrix4 *Matrix, f32 Width, f32 Height, f32 Left, f32 Top)
{
  static const f32 Far = 1.0f;
  static const f32 Near = 0.0f;

  Matrix->X0 = 2.0f / (Width - Left);
  Matrix->Y0 = 0.0f;
  Matrix->Z0 = 0.0f;
  Matrix->W0 = 0.0f;
  Matrix->X1 = 0.0f;
  Matrix->Y1 = 2.0f / (Top - Height);
  Matrix->Z1 = 0.0f;
  Matrix->W1 = 0.0f;
  Matrix->X2 = 0.0f;
  Matrix->Y2 = 0.0f;
  Matrix->Z2 = -2.0f / (Far - Near);
  Matrix->W2 = 0.0f;
  Matrix->X3 = -(Width + Left) / (Width - Left);
  Matrix->Y3 = -(Top + Height) / (Top - Height);
  Matrix->Z3 = -(Far + Near) / (Far - Near);
  Matrix->W3 = 1.0f;
}

inline void OrthographicMatrix(matrix4 *Matrix, f32 Width, f32 Height)
{
  OrthographicMatrix(Matrix, Width, Height, 0.0f, 0.0f);
}
