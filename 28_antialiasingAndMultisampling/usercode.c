#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "gl/gl_util.h"
#include "gl/gl_LTexture.h"

// -- section of variables for maintaining aspect ratio -- //
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

static bool g_need_clipping = false;

// -- section of variables for maintaining aspect ratio -- //
enum AliasMode
{
  // render with plain rasterization without smoothing out the edge
  ALIAS_MODE_ALIASED,
  // opengl will smooth out the edge
  ALIAS_MODE_ANTIALIASED,
  // multiple sample per pixel (another type of antialiased)
  ALIAS_MODE_MULTISAMPLE
};
static enum AliasMode aa_mode = ALIAS_MODE_ALIASED;

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
  glViewport(0, 0, g_screen_width, g_screen_height);

  // initialize projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, g_screen_width, g_screen_height, 0.0, -1.0, 1.0);

  // initialize modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // enable texturing
  glEnable(GL_TEXTURE_2D);
  // alpha blending doesn't work well with MSAA enabled like what we asked before creating a window in aa_msaa.c
  // even though we disable it with glDisable(GL_MULTISAMPLE)
  // possible way to work around this is to implement alpha blending in shader manually
  // see https://learnopengl.com/Advanced-OpenGL/Blending for reference
  glDisable(GL_BLEND);
  // disable depth teseting
  glDisable(GL_DEPTH_TEST);

  // set antialiasing / multisampling
  // note: setting hints are up to driver implementation whether it will listen to our request or not
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_POLYGON_SMOOTH);
  glDisable(GL_MULTISAMPLE);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error initializing OpenGL! %s", gluErrorString(error));
    return false;
  }

  return true;
}

bool usercode_loadmedia()
{
  // TODO: Load media here...
  return true;
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
        g_need_clipping = false;
        // set viewport
        glViewport(0, 0, g_screen_width, g_screen_height);
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
        g_need_clipping = true;
        // set viewport
        glViewport(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
      }
    }
    else if (k == SDLK_q)
    {
      // cycle alias mode
      aa_mode = (aa_mode + 1) % 3;

      if (aa_mode == ALIAS_MODE_ALIASED)
      {
        SDL_Log("Alias mode: Aliased");
      }
      else if (aa_mode == ALIAS_MODE_ANTIALIASED)
      {
        SDL_Log("Alias mode: Anti-aliased");
      }
      else if (aa_mode == ALIAS_MODE_MULTISAMPLE)
      {
        SDL_Log("Alias mode: Multisample");
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
  if (g_need_clipping)
    glClearColor(0.f, 0.f, 0.f, 1.f);
  else
    glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);

  // now clip content to be drawn only on content area (if needed)
  if (g_need_clipping)
  {
    // clear color for content area
    glEnable(GL_SCISSOR_TEST);
    glScissor(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  // we will work on modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  // set scale
  glScalef(g_ri_scale_x, g_ri_scale_y, 1.0f);

  // note: set viewport via glViewport accordingly, you should start at g_offset_x, and g_offset_y
  // note2: glViewport coordinate still in world coordinate, but for individual object (vertices) to be drawn, it's local coordinate

  // TODO: render code goes here...
  // start alias mode
  if (aa_mode == ALIAS_MODE_ALIASED)
  {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_MULTISAMPLE);
  }
  else if (aa_mode == ALIAS_MODE_ANTIALIASED)
  {
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glDisable(GL_MULTISAMPLE);
  }
  else if (aa_mode == ALIAS_MODE_MULTISAMPLE)
  {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glEnable(GL_MULTISAMPLE);
  }

  // render triangle
  glColor3f(1.f, 1.f, 1.f);
  glBegin(GL_TRIANGLES);
    glVertex2f(g_logical_width, 0.f);
    glVertex2f(g_logical_width, g_logical_height);
    glVertex2f(0.f, g_logical_height);
  glEnd();

  // end alias mode
  if (aa_mode == ALIAS_MODE_ANTIALIASED)
  {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
  }
  else if (aa_mode == ALIAS_MODE_MULTISAMPLE)
  {
    glDisable(GL_MULTISAMPLE);
  }

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{

}
