#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "gl/gl_util.h"

static int g_screen_width;
static int g_screen_height;

static int g_logical_width;
static int g_logical_height;
static float g_logical_aspect;

static int g_offset_x = 0;
static int g_offset_y = 0;
// resolution independent scale for x and y
static float g_ri_scale_x = 1.0f;
static float g_ri_scale_y = 1.0f;
// resolution independent dimensions
static int g_ri_view_width;
static int g_ri_view_height;

// unused for now
static float g_scale = 1.0;

// current viewport mode
static ViewportMode g_viewport_mode = VIEWPORT_MODE_FULL;

bool usercode_init(int screen_width, int screen_height, int logical_width, int logical_height)
{
  // FIXME: This code would works only if user starts with windowed mode, didnt test yet if start with fullscreen mode ...
  // set input screen dimensions
  g_screen_width = screen_width;
  g_screen_height = screen_height;

  g_logical_width = logical_width;
  g_logical_height = logical_height;
  g_logical_aspect = g_screen_width * 1.0f / g_screen_height;

  // start off with resolution matching the screen
  g_ri_view_width = g_screen_width;
  g_ri_view_height = g_screen_height;

  // initialize the viewport
  // define the area where to render, for now full screen
  glViewport(0.f, 0.f, g_screen_width, g_screen_height);

  // initialize projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, g_screen_width, g_screen_height, 0.0, -1.0, 1.0);

  // initialize modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // initialize clear color
  glClearColor(0.f, 1.f, 1.f, 1.f);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error initializing OpenGL! %s", gluErrorString(error));
    return false;
  }

  return true;
}

void usercode_set_global_scale(float scale)
{
  g_scale = scale;
}

void usercode_set_screen_dimension(Uint32 window_id, int screen_width, int screen_height)
{
  g_screen_width = screen_width;
  g_screen_height = screen_height;
}

void usercode_handle_event(SDL_Event *e, float delta_time)
{
  if (e->type == SDL_KEYDOWN)
  {
    int k = e->key.keysym.sym;

    // toggle fullscreen via enter key
    if (k == SDLK_RETURN)
    {
      // go windowed mode, currently in fullscreen mode
      if (gWindow->fullscreen)
      {
        LWindow_set_fullscreen(gWindow, false);
        // set projection matrix back to normal
        gl_util_adapt_to_normal(g_screen_width, g_screen_height);
        // reset relavant values back to normal
        g_offset_x = 0.0f;
        g_offset_y = 0.0f;
        g_ri_scale_x = 1.0f;
        g_ri_scale_y = 1.0f;
        g_ri_view_width = g_screen_width;
        g_ri_view_height = g_screen_height;
        SDL_Log("back to normal scale");
      }
      else
      {
        LWindow_set_fullscreen(gWindow, true);
        // get new window's size
        int w, h;
        SDL_GetWindowSize(gWindow->window, &w, &h);
        // also adapt to letterbox
        gl_util_adapt_to_letterbox(w, h, g_logical_width, g_logical_height, &g_ri_view_width, &g_ri_view_height, &g_offset_x, &g_offset_y);
        // calculate scale 
        g_ri_scale_x = g_ri_view_width * 1.0f / g_logical_width;
        g_ri_scale_y = g_ri_view_height * 1.0f / g_logical_height;

        SDL_Log("changed winsize to %d, %d with offset %d scale %f", w, h, g_offset_x, g_scale);
      }
    }
    // cycle through the available viewport mode
    else if (k == SDLK_q)
    {
      g_viewport_mode++;
      if (g_viewport_mode > VIEWPORT_MODE_RADAR)
      {
        g_viewport_mode = VIEWPORT_MODE_FULL;
      }
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
  //glTranslatef(view_width / 2.f + g_offset_x/2.f, view_height / 2.0f, 0.f);
  glTranslatef(g_ri_view_width / 2.f, g_ri_view_height / 2.f, 0.f);
  // set scale
  glScalef(g_ri_scale_x, g_ri_scale_y, 1.0f);

  // full view
  // note: no need to check for distortion of object here as its size depend on current screen resolution
  if (g_viewport_mode == VIEWPORT_MODE_FULL)
  {
    // fill the screen
    glViewport(g_offset_x, 0.0f, g_ri_view_width, g_ri_view_height);

    // red quad
    // note: render according to viewport, for vertex here we render 
    glBegin(GL_QUADS);
      glColor3f(1.f, 0.f, 0.f);
      glVertex2f(-g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, g_logical_height / 2.f);
      glVertex2f(-g_logical_width / 2.f, g_logical_height / 2.f);
    glEnd();
  }
  // half center
  // note: no need to check for distortion of object here as its size depend on current screen resolution
  else if (g_viewport_mode == VIEWPORT_MODE_HALF_CENTER)
  {
    // center but half size of the screen
    glViewport(g_ri_view_width / 4.0f + g_offset_x, g_ri_view_height / 4.0f, g_ri_view_width / 2.f, g_ri_view_height / 2.f);

    // green quad
    glBegin(GL_QUADS);
      glColor3f(0.f, 1.f, 0.f);
      glVertex2f(-g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, g_logical_height / 2.f);
      glVertex2f(-g_logical_width / 2.f, g_logical_height / 2.f);
    glEnd();
  }
  // half top
  // note: no need to check for distortion of object here as its size depend on current screen resolution
  else if (g_viewport_mode == VIEWPORT_MODE_HALF_TOP)
  {
    // set only half top of the screen
    glViewport(g_offset_x, g_ri_view_height / 2.f, g_ri_view_width, g_ri_view_height/2.0f);

    // blue quad
    glBegin(GL_QUADS);
      glColor3f(0.f, 0.f, 1.f);
      glVertex2f(-g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, g_logical_height / 2.f);
      glVertex2f(-g_logical_width / 2.f, g_logical_height / 2.f);
    glEnd();
  }
  // quad
  // note: no need to check for distortion of object here as its size depend on current screen resolution
  else if (g_viewport_mode == VIEWPORT_MODE_QUAD)
  {
    // top left, blue
    glViewport(g_ri_view_width / 8.0f + g_offset_x, g_ri_view_height / 8.f * 5.f, g_ri_view_width / 4.f, g_ri_view_height / 4.f);
    // blue quad
    glBegin(GL_QUADS);
      glColor3f(0.f, 0.f, 1.f);
      glVertex2f(-g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, g_logical_height / 2.f);
      glVertex2f(-g_logical_width / 2.f, g_logical_height / 2.f);
    glEnd();

    // top right, yellow
    glViewport(g_ri_view_width / 8.f * 5.f + g_offset_x, g_ri_view_height / 8.f * 5.f, g_ri_view_width / 4.f, g_ri_view_height / 4.f);
    // yellow quad
    glBegin(GL_QUADS);
      glColor3f(1.f, 1.f, 0.f);
      glVertex2f(-g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, g_logical_height / 2.f);
      glVertex2f(-g_logical_width / 2.f, g_logical_height / 2.f);
    glEnd();

    // bottom left, yellow
    glViewport(g_ri_view_width / 8.f + g_offset_x, g_ri_view_height / 8.f, g_ri_view_width / 4.f, g_ri_view_height / 4.f);
    // red quad
    glBegin(GL_QUADS);
      glColor3f(1.f, 0.f, 0.f);
      glVertex2f(-g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, g_logical_height / 2.f);
      glVertex2f(-g_logical_width / 2.f, g_logical_height / 2.f);
    glEnd();

    // bottom right, yellow
    glViewport(g_ri_view_width / 8.f * 5.f + g_offset_x, g_ri_view_height / 8.f, g_ri_view_width / 4.f, g_ri_view_height / 4.f);
    // gren quad
    glBegin(GL_QUADS);
      glColor3f(0.f, 1.f, 0.f);
      glVertex2f(-g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, -g_logical_height / 2.f);
      glVertex2f(g_logical_width / 2.f, g_logical_height / 2.f);
      glVertex2f(-g_logical_width / 2.f, g_logical_height / 2.f);
    glEnd();
  }
  // viewport with radar sub-viewport
  // note: check for distortion of object here when enter fullscreen (it's correct now!)
  else if (g_viewport_mode == VIEWPORT_MODE_RADAR)
  {
    const float size1 = 80.0f;
    const float size2 = 40.0f;

    // full size quad
    glViewport(g_offset_x, 0.0f, g_ri_view_width, g_ri_view_height);
    glBegin(GL_QUADS);
      glColor3f(1.f, 1.f, 1.f);
      glVertex2f(-size1, -size1);
      glVertex2f(size1, -size1);
      glVertex2f(size1, size1);
      glVertex2f(-size1, size1);

      glColor3f(0.f, 0.f, 0.f);
      glVertex2f(-size2, -size2);
      glVertex2f(size2, -size2);
      glVertex2f(size2, size2);
      glVertex2f(-size2, size2);
    glEnd();

    // radar quad
    glViewport(g_offset_x + g_ri_view_width / 2.f, g_ri_view_height / 2.f, g_ri_view_width / 2.f, g_ri_view_height / 2.f);
    glBegin(GL_QUADS);
      glColor3f(1.f, 1.f, 1.f);
      glVertex2f(-size1, -size1);
      glVertex2f(size1, -size1);
      glVertex2f(size1, size1);
      glVertex2f(-size1, size1);

      glColor3f(0.f, 0.f, 0.f);
      glVertex2f(-size2, -size2);
      glVertex2f(size2, -size2);
      glVertex2f(size2, size2);
      glVertex2f(-size2, size2);
    glEnd();
  }
}

void usercode_close()
{

}
