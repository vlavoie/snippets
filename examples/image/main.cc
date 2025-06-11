#include <common.hh>
#include <image/tga.hh>

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
layout (location = 0) in vec2 VertexPosition;
layout (location = 1) in vec2 VertexCoordinates;
layout (location = 2) in vec4 VertexColor;

uniform mat4 Projection;

out vec2 FragmentCoordinates;
out vec4 FragmentColor;

void main(void)
{
  FragmentCoordinates = VertexCoordinates;
  FragmentColor = VertexColor;
  gl_Position = Projection * vec4(VertexPosition.xy, 0.0, 1.0);
}
)";

const GLchar *FRAGMENT_SHADER = R"(
#version 330 core
in vec2 FragmentCoordinates;
in vec4 FragmentColor;

uniform sampler2D Texture0;

void main() {
  gl_FragColor = FragmentColor * texture(Texture0, FragmentCoordinates);
}
)";

static GLuint ShaderId, TextureId;

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
                                      GLsizei Length, const GLchar *Message, const void *userParam)
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
    fprintf(stdout, "\t%s Shader compiled.\n", Type == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex");
  }
  else
  {
    char buffer[4096];
    glGetShaderInfoLog(ShaderId, 4096, NULL, buffer);
    fprintf(stderr, "\t%s Shader failed to compile: %s\n",
            Type == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex", buffer);
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
    fprintf(stdout, "\tShader program linked.\n");
  }
  else
  {
    char buffer[4096];
    glGetProgramInfoLog(ShaderId, 4096, NULL, buffer);
    fprintf(stdout, "\tShader program failed linking: %s\n", buffer);
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

int main(int argc, char *argv[])
{
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

  WindowState.Window = XCreateWindow(
      WindowState.Display, WindowState.Root, 0, 0, 1024, 768, 0, WindowState.VisualInfo->depth,
      InputOutput, WindowState.VisualInfo->visual, CWColormap | CWEventMask, &SetWindowAttributes);

  XMapWindow(WindowState.Display, WindowState.Window);
  XStoreName(WindowState.Display, WindowState.Window, "Image Example");

  WindowState.GLContext =
      glXCreateContext(WindowState.Display, WindowState.VisualInfo, NULL, GL_TRUE);
  glXMakeCurrent(WindowState.Display, WindowState.Window, WindowState.GLContext);

  XWindowAttributes WindowAttributes;
  XGetWindowAttributes(WindowState.Display, WindowState.Window, &WindowAttributes);
  WindowState.Width = WindowAttributes.width;
  WindowState.Height = WindowAttributes.height;

  Atom WindowDeleteMessage = XInternAtom(WindowState.Display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(WindowState.Display, WindowState.Window, &WindowDeleteMessage, 1);
  XEvent WindowEvent;

  bool32 Running = true;
  while (Running)
  {
    XNextEvent(WindowState.Display, &WindowEvent);

    if (WindowEvent.type == ClientMessage && WindowEvent.xclient.data.l[0] == WindowDeleteMessage)
    {
      Running = false;
    }
  }

  fprintf(stdout, "Closing x11 state.\n");
  if (glXMakeCurrent(WindowState.Display, None, NULL))
  {
    fprintf(stdout, "\tShutdown GLX context.\n");
  }
  else
  {
    fprintf(stderr, "\tFailed to shutdown GLX context\n");
    return 1;
  }

  glXDestroyContext(WindowState.Display, WindowState.GLContext);

  if (XDestroyWindow(WindowState.Display, WindowState.Window))
  {
    fprintf(stdout, "\tDestroying x11 window.\n");
  }
  else
  {
    fprintf(stderr, "\tFailed to destroy x11 window.\n");
    return 1;
  }

  if (XDestroyWindow(WindowState.Display, WindowState.Root))
  {
    fprintf(stdout, "\tDestroying x11 root window.\n");
  }
  else
  {
    fprintf(stderr, "\tFailed to destroy x11 root window.\n");
    return 1;
  }

  fprintf(stdout, "\tClosing x11 display.\n");
  XCloseDisplay(WindowState.Display);

  return 0;
}
