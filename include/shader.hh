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
