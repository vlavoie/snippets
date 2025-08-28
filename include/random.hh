/*
Header for randomization functions and utilities.
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

#include <common.hh>

#ifndef __RANDOM__Rand
#include <stdlib.h>
#define __RANDOM__Rand rand
#endif

struct shift_register
{
  u32 Seed;
};

inline shift_register CreateShiftRegister()
{
  return {.Seed = u32(u16(rand()) << 16) | (u16(rand()))};
}

inline u32 ShiftRegisterSeed(shift_register *ShiftRegister)
{
  if (ShiftRegister->Seed & 1)
  {
    ShiftRegister->Seed = (ShiftRegister->Seed >> 1) ^ 0x80000062;
  }
  else
  {
    ShiftRegister->Seed >>= 1;
  }

  return ShiftRegister->Seed;
};

inline u32 XorShiftRegisterSeed(shift_register *ShiftRegister)
{
  ShiftRegister->Seed ^= ShiftRegister->Seed << 13;
  ShiftRegister->Seed ^= ShiftRegister->Seed >> 7;
  ShiftRegister->Seed ^= ShiftRegister->Seed << 17;

  return ShiftRegister->Seed;
};
