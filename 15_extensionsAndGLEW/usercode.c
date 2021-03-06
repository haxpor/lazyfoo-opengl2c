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
static gl_LTexture* repeating_texture = NULL;
static float tex_x = 0.f;
static float tex_y = 0.f;
static int texture_wrap_type = 0;

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
  repeating_texture = gl_LTexture_new();
  if (!gl_LTexture_load_texture_from_file(repeating_texture, "texture.png"))
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
      texture_wrap_type++;
      if (texture_wrap_type >= 5)
      {
        texture_wrap_type = 0;
      }

      // bind to repeating texture
      glBindTexture(GL_TEXTURE_2D, repeating_texture->texture_id);

      if (texture_wrap_type == 0)
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        SDL_Log("Set to GL_REPEAT");
      }
      else if (texture_wrap_type == 1)
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        SDL_Log("Set to GL_CLAMP");
      }
      else if (texture_wrap_type == 2)
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        SDL_Log("Set to GL_CLAMP_TO_BORDER");
      }
      else if (texture_wrap_type == 3)
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        SDL_Log("Set to GL_CLAMP_TO_EDGE");
      }
      else if (texture_wrap_type == 4)
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        SDL_Log("Set to GL_MIRRORED_REPEAT");
      }

      // unbind
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}

void usercode_update(float delta_time)
{
  // scroll texture
  tex_x++;
  tex_y++;

  // cap scrolling
  if (tex_x >= repeating_texture->width)
  {
    tex_x = 0.f;
  }
  if (tex_y >= repeating_texture->height)
  {
    tex_y = 0.f;
  }
}

void usercode_render()
{
  // clear color buffer
  if (g_need_clipping)
    glClearColor(0.f, 0.f, 0.f, 1.f);
  else
    glClearColor(1.f, 1.f, 1.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);

  // now clip content to be drawn only on content area (if needed)
  if (g_need_clipping)
  {
    // clear color for content area
    glEnable(GL_SCISSOR_TEST);
    glScissor(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
    glClearColor(1.f, 1.f, 1.f, 1.f);
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
  // calculate texture maximma
  GLfloat texture_right = (GLfloat)g_logical_width / repeating_texture->width;
  GLfloat texture_bottom = (GLfloat)g_logical_height / repeating_texture->height;

  // use repeating texture
  glBindTexture(GL_TEXTURE_2D, repeating_texture->texture_id);

  // switch to texture matrix
  glMatrixMode(GL_TEXTURE);

  // reset transformation
  glLoadIdentity();

  // scroll texture
  glTranslatef(tex_x / repeating_texture->width, tex_y / repeating_texture->height, 0.f);

  // render
  glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f);                       glVertex2f(0.f, 0.f);
    glTexCoord2f(texture_right, 0.f);             glVertex2f(g_logical_width, 0.f);
    glTexCoord2f(texture_right, texture_bottom);  glVertex2f(g_logical_width, g_logical_height);
    glTexCoord2f(0.f, texture_bottom);            glVertex2f(0.f, g_logical_height);
  glEnd();

  // unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{
  if (repeating_texture != NULL)
    gl_LTexture_free(repeating_texture);
}
