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
static gl_LTexture* texture = NULL;
// polygon attributes
static GLfloat polygon_angle = 0.f;
static GLfloat polygon_x; // will be filled at loadmedia
static GLfloat polygon_y; // will be filled at loadmedia
// stencil operation
static GLuint stencil_render_op = GL_NOTEQUAL;
// 0 for triangle, and 1 for rectangle
static int stencil_polygon_type = 0;

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
  // set clearing color used by stencil buffer
  glClearStencil(0);

  // enable texturing
  glEnable(GL_TEXTURE_2D);
  // disable depth testing
  glDisable(GL_DEPTH_TEST);

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
  polygon_x = g_logical_width / 2.f;
  polygon_y = g_logical_height / 2.f;
  
  texture = gl_LTexture_new();
  if (!gl_LTexture_load_texture_from_file(texture, "opengl.png"))
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
    else if (k == SDLK_w)
    {
      // cycle between polygon option for rendering onto stencil buffer
      stencil_polygon_type = (stencil_polygon_type + 1) % 2;
    }
    else if (k == SDLK_q)
    {
      // cycle through stencil operation
      if (stencil_render_op == GL_NOTEQUAL)
      {
        // render where stencil polygon was rendered
        stencil_render_op = GL_EQUAL;
      }
      else if (stencil_render_op == GL_EQUAL)
      {
        // render everything
        stencil_render_op = GL_ALWAYS;
      }
      else if (stencil_render_op == GL_ALWAYS)
      {
        // render where stencil polygon wasn't rendered
        stencil_render_op = GL_NOTEQUAL;
      }
    }
  }
  else if (e->type == SDL_MOUSEMOTION)
  {
    // update polygon's position according to mouse's position
    SDL_MouseMotionEvent motion = e->motion;
    // check whether we need to apply scale if user goes full screen
    if (gWindow->fullscreen)
    {
      polygon_x = (motion.x - g_offset_x) / g_ri_scale_x;
      polygon_y = (motion.y - g_offset_y) / g_ri_scale_y;
    }
    else
    {
      polygon_x = motion.x;
      polygon_y = motion.y;
    }
  }
}

void usercode_update(float delta_time)
{
  polygon_angle += 6.f;
  if (polygon_angle >= 360.f)
  {
    polygon_angle = -360.0f;
  }
}

void usercode_render()
{
  // clear color buffer
  if (g_need_clipping)
    glClearColor(0.f, 0.f, 0.f, 1.f);
  else
    glClearColor(0.f, 0.f, 1.f, 1.f);
  // now we clear both color and stencil buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // now clip content to be drawn only on content area (if needed)
  if (g_need_clipping)
  {
    // clear color for content area
    glEnable(GL_SCISSOR_TEST);
    glScissor(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
    glClearColor(0.f, 0.f, 1.f, 1.f);
    // now we clear both color and stencil buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }
  
  // we will work on modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  // set scale
  glScalef(g_ri_scale_x, g_ri_scale_y, 1.0f);

  // note: set viewport via glViewport accordingly, you should start at g_offset_x, and g_offset_y
  // note2: glViewport coordinate still in world coordinate, but for individual object (vertices) to be drawn, it's local coordinate

  // TODO: render code goes here...
  // -- render mask into stencil buffer
  // disable rendering to the color buffer
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  // start using stencil buffer
  glEnable(GL_STENCIL_TEST);
  
  // place a 1 where rendered
  glStencilFunc(GL_ALWAYS, 1, 1);
  // replace when rendered
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

  // render stencil triangle
  glPushMatrix();
  glTranslatef(polygon_x, polygon_y, 0.f);
  glRotatef(polygon_angle, 0.f, 0.f, 1.f);

  if (stencil_polygon_type == 0)
  {
    // render triangle
    glBegin(GL_TRIANGLES);
      glVertex2f(-0.f / 4.f, -g_logical_height / 4.f);
      glVertex2f(g_logical_width / 4.f, g_logical_height / 4.f);
      glVertex2f(-g_logical_width / 4.f, g_logical_height / 4.f);
    glEnd();
  }
  else if (stencil_polygon_type == 1)
  {
    // render quads
    glBegin(GL_QUADS);
      glVertex2f(-g_logical_width / 4.f, -g_logical_height / 4.f);
      glVertex2f(g_logical_width / 4.f, -g_logical_height / 4.f);
      glVertex2f(g_logical_width / 4.f, g_logical_height / 4.f);
      glVertex2f(-g_logical_width / 4.f, g_logical_height / 4.f);
    glEnd();
  }
  glPopMatrix();

  // -- render real stuff into color buffer
  // reenable color
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // where a 1 was not rendered
  glStencilFunc(stencil_render_op, 1, 1);

  // keep the pixel
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  // -- done with color buffer and stencil buffer
  gl_LTexture_render(texture, (g_logical_width - texture->width)/2.f, (g_logical_height - texture->height)/2.f, NULL);

  // finish using stencil
  glDisable(GL_STENCIL_TEST);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{
  if (texture != NULL)
    gl_LTexture_free(texture);
}
