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

void FreeShaderProgram(key ShaderID)
{
  glDeleteProgram(ShaderID);
}

void UseShaderProgram(key ShaderID)
{
  glUseProgram(ShaderID);
}

void SetShaderUniformInteger(key ShaderID, const char *UniformName, i32 Value)
{
  glUniform1i(glGetUniformLocation(ShaderID, UniformName), Value);
}

void SetShaderUniformUnsigned(key ShaderID, const char *UniformName, u32 Value)
{
  glUniform1ui(glGetUniformLocation(ShaderID, UniformName), Value);
}

void SetShaderUniformFloat(key ShaderID, const char *UniformName, f32 Value)
{
  glUniform1f(glGetUniformLocation(ShaderID, UniformName), Value);
}

void SetShaderUniformMatrix4(key ShaderID, const char *UniformName, matrix4 *Matrix)
{
  glUniformMatrix4fv(glGetUniformLocation(ShaderID, UniformName), 1, GL_FALSE, &Matrix->X0);
}
