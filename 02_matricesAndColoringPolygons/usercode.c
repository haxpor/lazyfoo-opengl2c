#include "usercode.h"

static int g_screen_width;
static int g_screen_height;

// the current color rendering mode
static ColorMode g_colormode = COLORMODE_CYAN;

// the projection scale
// control how large of a coordinate area we want to render
static GLfloat g_project_scale = 1.f;

bool usercode_init(int screen_width, int screen_height)
{
  // set input screen dimensions
  g_screen_width = screen_width;
  g_screen_height = screen_height;

  // initialize projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, g_screen_width, g_screen_height, 0.0, 1.0, -1.0);

  // initialize modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error initializing OpenGL! %s", gluErrorString(error));
    return false;
  }

  return true;
}

void usercode_handle_event(SDL_Event *e, float delta_time)
{
  if (e->type == SDL_KEYDOWN)
  {
    int k = e->key.keysym.sym;

    // toggle mode bettern cyan and multi
    if (k == SDLK_q)
    {
      if (g_colormode == COLORMODE_CYAN)
      {
        g_colormode = COLORMODE_MULTI;
        SDL_Log("changed to multi color mode");
      }
      else
      {
        g_colormode = COLORMODE_CYAN;
        SDL_Log("changed to cyan color mode");
      }
    }
    // cycle through projection scales
    else if (k == SDLK_e)
    {
      if (g_project_scale == 1.f)
      {
        // zoom out
        g_project_scale = 2.f;
        SDL_Log("projection scale = 2.f");
      }
      else if (g_project_scale == 2.f)
      {
        // zoom in
        g_project_scale = 0.5f;
        SDL_Log("projection scale = 0.5f");
      }
      else if (g_project_scale == 0.5f)
      {
        // regular zoom
        g_project_scale = 1.f;
        SDL_Log("projection scale = 1.f");
      }

      // update projection matrix
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      // we can as well multiple scale to projection matrix
      glOrtho(0.0, g_screen_width * g_project_scale, g_screen_height * g_project_scale, 0.0, 1.0, -1.0);
    }
  }
}

void usercode_update(float delta_time)
{

}

void usercode_render()
{
  // clear color buffer
  glClear(GL_COLOR_BUFFER_BIT);

  // we will work on modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // move to center of the screen
  glTranslatef(g_screen_width * g_project_scale / 2.f, g_screen_height * g_project_scale / 2.f, 0.f);

  // render quad
  if (g_colormode == COLORMODE_CYAN)
  {
    glBegin(GL_QUADS);
      glColor3f(0.f, 1.f, 1.f);
      glVertex2f(-50.f, -50.f);
      glVertex2f(50.f, -50.f);
      glVertex2f(50.f, 50.f);
      glVertex2f(-50.f, 50.f);
    glEnd();
  }
  else
  {
    // RYGB mix
    glBegin(GL_QUADS);
      glColor3f(1.f, 0.f, 0.f); glVertex2f(-50.f, -50.f);
      glColor3f(1.f, 1.f, 0.f); glVertex2f(50.f, -50.f);
      glColor3f(0.f, 1.f, 0.f); glVertex2f(50.f, 50.f);
      glColor3f(0.f, 0.f, 1.f); glVertex2f(-50.f, 50.f);
    glEnd();
  }
}

void usercode_close()
{

}
