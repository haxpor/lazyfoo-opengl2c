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
static gl_LTexture* rotating_texture = NULL;
static GLfloat rot_angle = 0.f;
// indicate which transformation to be used
static int transformation_combo = 0;

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
  rotating_texture = gl_LTexture_new();
  if (!gl_LTexture_load_texture_from_file(rotating_texture, "texture.png"))
  {
    SDL_Log("Unable to load texture");
    return false;
  }
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
    // q is pressed
    else if (k == SDLK_q)
    {
      // reset rotation
      rot_angle = 0.f;

      // cycle through transformation combos
      transformation_combo++;
      if (transformation_combo > 4)
      {
        transformation_combo = 0;
      }
    }
  }
}

void usercode_update(float delta_time)
{
  rot_angle += 1.f;
  // cap angle
  if (rot_angle >= 360.0f)
  {
    rot_angle -= 360.0f;
  }
}

void usercode_render()
{
  // clear color buffer
  if (g_need_clipping)
    glClearColor(0.f, 0.f, 0.f, 1.f);
  else
    glClearColor(1.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);

  // now clip content to be drawn only on content area (if needed)
  if (g_need_clipping)
  {
    // clear color for content area
    glEnable(GL_SCISSOR_TEST);
    glScissor(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
    glClearColor(1.f, 0.f, 0.f, 1.f);
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
  if (transformation_combo == 0)
  {
    // to rotate object around itself, see https://www.opengl.org/discussion_boards/showthread.php/182127-Rotating-quad-around-itself-but-move-it-to-a-specific-position-of-another-quad
    glTranslatef(g_logical_width / 2.f, g_logical_height / 2.f, 0.f);
    glRotatef(rot_angle, 0.f, 0.f, 1.f);
    glScalef(2.f, 2.f, 0.f);
    glTranslatef(rotating_texture->width / -2.f, rotating_texture->height / -2.f, 0.f);
  }
  else if (transformation_combo == 1)
  {
    // this one rotation is off as we scale it as the last operation
    glTranslatef(g_logical_width / 2.f, g_logical_height / 2.f, 0.f);
    glRotatef(rot_angle, 0.f, 0.f, 1.f);
    glTranslatef(rotating_texture->width / -2.f, rotating_texture->height / -2.f, 0.f);
    glScalef(2.f, 2.f, 0.f);
  }
  else if (transformation_combo == 2)
  {
    // scale first which affects it to rotates around the bottom right corner of the screen
    glScalef(2.f, 2.f, 0.f);
    glTranslatef(g_logical_width / 2.f, g_logical_height / 2.f, 0.f);
    glRotatef(rot_angle, 0.f, 0.f, 1.f);
    glTranslatef(rotating_texture->width / -2.f, rotating_texture->height / -2.f, 0.f);
  }
  else if (transformation_combo == 3)
  {
    // note translate half of texture thus it rotates around its origin (top left)
    glTranslatef(g_logical_width / 2.f, g_logical_height / 2.f, 0.f);
    glRotatef(rot_angle, 0.f, 0.f, 1.f);
    glScalef(2.f, 2.f, 0.f);
  }
  else if (transformation_combo == 4)
  {
    // rotate first, cause the quad to rotate around top left corner (not exact position as we scale it last) of the screen
    glRotatef(rot_angle, 0.f, 0.f, 1.f);
    glTranslatef(g_logical_width / 2.f, g_logical_height / 2.f, 0.f);
    glScalef(2.f, 2.f, 0.f);
    glTranslatef(rotating_texture->width / -2.f, rotating_texture->height / -2.f, 0.f);
  }

  // render
  gl_LTexture_render(rotating_texture, 0.f, 0.f, NULL);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{
  if (rotating_texture != NULL)
    gl_LTexture_free(rotating_texture);
}
