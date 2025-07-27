/*
Example program to use tga.hh
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

#include <common.hh>
#include <tga.hh>

// OpenGL
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
// x11
#include <X11/X.h>
#include <X11/Xlib.h>
// glibc
#include <stdio.h>
#include <stdlib.h>

// gl variables
const GLchar *VERTEX_SHADER = R"(
#version 330 core

layout (location = 0) in vec2 VertexCoordinate;
layout (location = 1) in vec2 TextureCoordinate;

out vec2 FragmentCoordinate;

void main(void)
{
  FragmentCoordinate = TextureCoordinate;
  gl_Position = vec4(VertexCoordinate.xy, 0.0, 1.0);
}
)";

const GLchar *FRAGMENT_SHADER = R"(
#version 330 core
in vec2 FragmentCoordinate;

uniform sampler2D Texture0;

void main() {
  gl_FragColor = texture(Texture0, FragmentCoordinate);
}
)";

struct vec2
{
  f32 X, Y;
};

struct vertex
{
  vec2 Coordinate;
  vec2 TextureCoordinate;
};

static vertex Vertices[6] = {
    // top-left
    {
        .Coordinate =
            {
                .X = -1.0f,
                .Y = 1.0f,
            },
        .TextureCoordinate =
            {
                .X = 0.0f,
                .Y = 0.0f,
            },
    },
    // top-right
    {
        .Coordinate =
            {
                .X = 1.0f,
                .Y = 1.0f,
            },
        .TextureCoordinate =
            {
                .X = 1.0f,
                .Y = 0.0f,
            },
    },
    // bottom-left
    {
        .Coordinate =
            {
                .X = -1.0f,
                .Y = -1.0f,
            },
        .TextureCoordinate =
            {
                .X = 0.0f,
                .Y = 1.0f,
            },
    },
    // top-right
    {
        .Coordinate =
            {
                .X = 1.0f,
                .Y = 1.0f,
            },
        .TextureCoordinate =
            {
                .X = 1.0f,
                .Y = 0.0f,
            },
    },
    // bottom-left
    {
        .Coordinate =
            {
                .X = -1.0f,
                .Y = -1.0f,
            },
        .TextureCoordinate =
            {
                .X = 0.0f,
                .Y = 1.0f,
            },
    },
    // bottom-right
    {
        .Coordinate =
            {
                .X = 1.0f,
                .Y = -1.0f,
            },
        .TextureCoordinate =
            {
                .X = 1.0f,
                .Y = 1.0f,
            },
    },
};

static GLuint ShaderId, TextureId, VBO, VAO;

// x11 variables
struct x_state
{
  i32 Screen;
  i32 Width;
  i32 Height;
  Window Root;
  Window Window;
  GLXContext GLContext;
  Display *Display;
  XVisualInfo *VisualInfo;
};

static x_state WindowState;

void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum Type, GLuint Id, GLenum Severity,
                                      GLsizei Length, const GLchar *Message, const void *UserParam)
{
  fprintf(stderr, "GL CALLBACK: %s Type = 0x%x, Severity = 0x%x, Message = %s\n",
          (Type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), Type, Severity, Message);
}

GLuint CompileShader(const GLchar *const *ShaderSource, GLenum Type)
{
  GLuint ShaderId = glCreateShader(Type);
  glShaderSource(ShaderId, 1, ShaderSource, NULL);
  glCompileShader(ShaderId);

  i32 CompileStatue;
  glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &CompileStatue);
  if (CompileStatue)
  {
    fprintf(stdout, "%s Shader compiled.\n", Type == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex");
  }
  else
  {
    char Buffer[4096];
    glGetShaderInfoLog(ShaderId, 4096, NULL, Buffer);
    fprintf(stderr, "%s Shader failed to compile: %s\n",
            Type == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex", Buffer);
  }

  return ShaderId;
}

GLuint CreateShaderProgram(const GLchar *const *VertexSource, const GLchar *const *FragmentSource)
{
  GLuint ShaderId = glCreateProgram();

  GLuint VertexShader = CompileShader(VertexSource, GL_VERTEX_SHADER);
  GLuint FragmentShader = CompileShader(FragmentSource, GL_FRAGMENT_SHADER);

  glAttachShader(ShaderId, VertexShader);
  glAttachShader(ShaderId, FragmentShader);
  glLinkProgram(ShaderId);

  GLint LinkingStatus;
  glGetProgramiv(ShaderId, GL_LINK_STATUS, &LinkingStatus);
  if (LinkingStatus)
  {
    fprintf(stdout, "Shader program linked.\n");
  }
  else
  {
    char Buffer[4096];
    glGetProgramInfoLog(ShaderId, 4096, NULL, Buffer);
    fprintf(stdout, "Shader program failed linking: %s\n", Buffer);
  }

  glDeleteShader(VertexShader);
  glDeleteShader(FragmentShader);
  return ShaderId;
}

i32 HandleX11Error(Display *XDisplay, XErrorEvent *XError)
{
  char Message[256];
  if (XGetErrorText(XDisplay, XError->error_code, Message, sizeof(Message)))
  {
    fprintf(stderr, "Failed to parse error message %d", XError->error_code);
    return 1;
  }
  fprintf(stderr, "X11 error code %d: %s", XError->error_code, Message);

  return 0;
}

i32 main(i32 Argc, char *Argv[])
{
  if (Argc < 2)
  {
    fprintf(stderr, "TGA file argument is required to render image.");
    return 1;
  }

  FILE *File = fopen(Argv[1], "r");
  key TGALength = 0;
  void *TGAData = 0x0;

  if (File)
  {
    fseek(File, 0, SEEK_END);
    TGALength = ftell(File);
    TGAData = malloc(sizeof(byte) * TGALength);
    rewind(File);
    fread(TGAData, 1, TGALength, File);

    fclose(File);
  }
  else
  {
    fprintf(stderr, "Failed to read TGA file.");
    return 1;
  }

  tga::texture *Texture = tga::Decompress(TGALength, TGAData);

  if (Texture == 0x0)
  {
    fprintf(stderr, "Failed to parse TGA file.");
    return 1;
  }

  // x11 state initialization
  fprintf(stdout, "Initializing x11 platform.\n");
  GLint GLAttributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
  Colormap XColorMap;

  XSetWindowAttributes SetWindowAttributes;

  XSetErrorHandler(&HandleX11Error);
  WindowState.Display = XOpenDisplay(NULL);

  if (!WindowState.Display)
  {
    fprintf(stderr, "Failed to connect to x11 server.\n");
    return 1;
  }

  WindowState.Screen = DefaultScreen(WindowState.Display);
  WindowState.Root = DefaultRootWindow(WindowState.Display);
  WindowState.VisualInfo = glXChooseVisual(WindowState.Display, 0, GLAttributes);

  if (!WindowState.VisualInfo)
  {
    fprintf(stderr, "No appropriate display found\n");
    return 1;
  }
  else
  {
    fprintf(stdout, "Display %p selected\n", (void *)WindowState.VisualInfo->visualid);
  }

  XColorMap = XCreateColormap(WindowState.Display, WindowState.Root, WindowState.VisualInfo->visual,
                              AllocNone);
  SetWindowAttributes.colormap = XColorMap;
  SetWindowAttributes.event_mask = ExposureMask;

  WindowState.Window =
      XCreateWindow(WindowState.Display, WindowState.Root, 0, 0, Texture->Width, Texture->Height, 0,
                    WindowState.VisualInfo->depth, InputOutput, WindowState.VisualInfo->visual,
                    CWColormap | CWEventMask, &SetWindowAttributes);

  XMapWindow(WindowState.Display, WindowState.Window);
  XStoreName(WindowState.Display, WindowState.Window, "Image Example");

  WindowState.GLContext =
      glXCreateContext(WindowState.Display, WindowState.VisualInfo, NULL, GL_TRUE);
  glXMakeCurrent(WindowState.Display, WindowState.Window, WindowState.GLContext);

  XWindowAttributes WindowAttributes;
  XGetWindowAttributes(WindowState.Display, WindowState.Window, &WindowAttributes);
  WindowState.Width = WindowAttributes.width;
  WindowState.Height = WindowAttributes.height;

  ShaderId = CreateShaderProgram(&VERTEX_SHADER, &FRAGMENT_SHADER);
  glUseProgram(ShaderId);

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(OpenGLMessageCallback, 0);

  glViewport(0, 0, Texture->Width, Texture->Height);
  glClearColor(0, 0, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)sizeof(vec2));
  glEnableVertexAttribArray(1);

  glGenTextures(1, &TextureId);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, TextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 6, Vertices, GL_STATIC_DRAW);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture->Width, Texture->Height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, Texture->Pixels);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glXSwapBuffers(WindowState.Display, WindowState.Window);

  Atom WindowDeleteMessage = XInternAtom(WindowState.Display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(WindowState.Display, WindowState.Window, &WindowDeleteMessage, 1);
  XEvent WindowEvent;

  while (true)
  {
    XNextEvent(WindowState.Display, &WindowEvent);

    if (WindowEvent.type == ClientMessage && WindowEvent.xclient.data.l[0] == WindowDeleteMessage)
    {
      break;
    }
  }

  fprintf(stdout, "Closing x11 state.\n");
  if (glXMakeCurrent(WindowState.Display, None, NULL))
  {
    fprintf(stdout, "Shutdown GLX context.\n");
  }
  else
  {
    fprintf(stderr, "Failed to shutdown GLX context\n");
    return 1;
  }

  glXDestroyContext(WindowState.Display, WindowState.GLContext);

  if (XDestroyWindow(WindowState.Display, WindowState.Window))
  {
    fprintf(stdout, "Destroying x11 window.\n");
  }
  else
  {
    fprintf(stderr, "Failed to destroy x11 window.\n");
    return 1;
  }

  if (XDestroyWindow(WindowState.Display, WindowState.Root))
  {
    fprintf(stdout, "Destroying x11 root window.\n");
  }
  else
  {
    fprintf(stderr, "Failed to destroy x11 root window.\n");
    return 1;
  }

  fprintf(stdout, "Closing x11 display.\n");
  XCloseDisplay(WindowState.Display);

  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteTextures(1, &TextureId);
  glDeleteProgram(ShaderId);

  free(TGAData);

  return 0;
}
