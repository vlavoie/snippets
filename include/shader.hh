/*
Header for shaders
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
#include "math2d.hh"

key CompileShader(const char *const *ShaderSource, key Type);
key CreateShaderProgram(const char *const *VsSource, const char *const *FsSource);
void FreeShaderProgram(key ShaderID);

void UseShaderProgram(key ShaderID);

void SetShaderUniformInteger(key ShaderID, const char *UniformName, i32 Value);
void SetShaderUniformUnsigned(key ShaderID, const char *UniformName, u32 Value);
void SetShaderUniformFloat(key ShaderID, const char *UniformName, f32 Value);
void SetShaderUniformMatrix4(key ShaderID, const char *UniformName, matrix4 *Matrix);
