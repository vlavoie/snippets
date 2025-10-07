/*
Implementation for shaders
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

#include "shader.hh"

// libc
#include <stdio.h>
// OpenGL
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

key CompileShader(const char *const *ShaderSource, key Type)
{
  GLuint ShaderID = glCreateShader(Type);
  glShaderSource(ShaderID, 1, ShaderSource, NULL);
  glCompileShader(ShaderID);

  GLint CompileStatus;
  glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileStatus);

  if (CompileStatus)
  {
    fprintf(stdout, "%s Shader compiled.\n", Type == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex");
  }
  else
  {
    char Buffer[4096];
    glGetShaderInfoLog(ShaderID, 4096, NULL, Buffer);
    fprintf(stdout, "%s Shader failed to compile: %s\n",
            Type == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex", Buffer);
  }

  return ShaderID;
}

key CreateShaderProgram(const char *const *VertexSource, const char *const *FragmentSource)
{
  GLuint ShaderProgram = glCreateProgram();

  GLuint VertexShader = CompileShader(VertexSource, GL_VERTEX_SHADER);
  GLuint FragmentShader = CompileShader(FragmentSource, GL_FRAGMENT_SHADER);

  glAttachShader(ShaderProgram, VertexShader);
  glAttachShader(ShaderProgram, FragmentShader);
  glLinkProgram(ShaderProgram);

  GLint LinkingStatus;
  glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &LinkingStatus);
  if (LinkingStatus)
  {
    fprintf(stdout, "Shader program linked.\n");
  }
  else
  {
    char Buffer[4096];
    glGetProgramInfoLog(ShaderProgram, 4096, NULL, Buffer);
    fprintf(stdout, "Shader program failed linking: %s\n", Buffer);
  }

  glDeleteShader(VertexShader);
  glDeleteShader(FragmentShader);
  return ShaderProgram;
}

void FreeShaderProgram(const key ShaderID)
{
  glDeleteProgram(ShaderID);
}

void UseShaderProgram(const key ShaderID)
{
  glUseProgram(ShaderID);
}

void SetShaderUniformInteger(const key ShaderID, const char *UniformName, const i32 Value)
{
  glUniform1i(glGetUniformLocation(ShaderID, UniformName), Value);
}

void SetShaderUniformUnsigned(const key ShaderID, const char *UniformName, const u32 Value)
{
  glUniform1ui(glGetUniformLocation(ShaderID, UniformName), Value);
}

void SetShaderUniformFloat(const key ShaderID, const char *UniformName, const f32 Value)
{
  glUniform1f(glGetUniformLocation(ShaderID, UniformName), Value);
}

void SetShaderUniformMatrix4(const key ShaderID, const char *UniformName, const matrix4 *Matrix)
{
  glUniformMatrix4fv(glGetUniformLocation(ShaderID, UniformName), 1, GL_FALSE, &Matrix->X0);
}

void SetShaderUniformSampler(const key ShaderID, const char *UniformName, const key SamplerID)
{
  SetShaderUniformInteger(ShaderID, UniformName, i32(SamplerID));
}

void SetShaderUniformColor(const key ShaderID, const char *UniformName, const color Color)
{
  rgba RGBA = UnpackRGBA(Color);
  glUniform4fv(glGetUniformLocation(ShaderID, UniformName), 1, &RGBA.R);
}
