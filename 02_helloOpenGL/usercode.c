#include "usercode.h"

static int g_screen_width;
static int g_screen_height;

bool usercode_init(int screen_width, int screen_height)
{
  // set input screen dimensions
  g_screen_width = screen_width;
  g_screen_height = screen_height;

  // initialize projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // initialize modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    printf("Error initializing OpenGL! %s", gluErrorString(error));
    return false;
  }

  return true;
}

void usercode_update(float delta_time)
{

}

void usercode_render()
{
  // clear color buffer
  glClear(GL_COLOR_BUFFER_BIT);

  // render quad
  glBegin(GL_QUADS);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(0.5f, -0.5f);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(-0.5f, 0.5f);
  glEnd();
}

void usercode_handle_event(SDL_Event *e, float delta_time)
{

}

void usercode_close()
{

}
