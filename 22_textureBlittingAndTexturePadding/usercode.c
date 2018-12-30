#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "gl/gl_util.h"
#include "gl/gl_LTexture.h"
#include "gl/gl_LTexture_internals.h"

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
// generated combined texture
static gl_LTexture* combined_texture = NULL;
static gl_LTexture* combined_grayscale_texture = NULL;

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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
  // load left texture as pixels as we will get pixels data from, so we don't have to lock
  // 1. 32-bit PNG file format
  {
  gl_LTexture* left_texture = gl_LTexture_new();
  if (!gl_LTexture_load_pixels_from_file(left_texture, "left.png"))
  {
    SDL_Log("Unable to load left texture");
    return false;
  }

  // load right texture
  gl_LTexture* right_texture = gl_LTexture_new();
  if (!gl_LTexture_load_pixels_from_file(right_texture, "right.png"))
  {
    SDL_Log("Unable to load right texture");
    return false;
  }

  // create a blank pixels to combine both left and right texture
  combined_texture = gl_LTexture_new();
  gl_LTexture_create_pixels32(combined_texture, left_texture->width + right_texture->width, left_texture->height);

  // blit image
  gl_LTexture_blit_pixels32(left_texture, 0, 0, combined_texture);
  gl_LTexture_blit_pixels32(right_texture, left_texture->width, 0, combined_texture);

  // pad and create texture
  gl_LTexture_pad_pixels32(combined_texture);
  gl_LTexture_load_texture_from_precreated_pixels32(combined_texture);

  // get rid of old textures
  gl_LTexture_free(left_texture);
  gl_LTexture_free(right_texture);
  }

  // 2. 8-bit Grayscale file format
  {
  gl_LTexture* left_texture = gl_LTexture_new();
  if (!gl_LTexture_load_pixels_from_file8(left_texture, "left-grayscale.png"))
  {
    SDL_Log("Unable to load left-grayscale texture");
    return false;
  }

  // load right texture
  gl_LTexture* right_texture = gl_LTexture_new();
  if (!gl_LTexture_load_pixels_from_file8(right_texture, "right-grayscale.png"))
  {
    SDL_Log("Unable to load right-grayscale texture");
    return false;
  }

  // create a blank pixels to combine both left and right texture
  combined_grayscale_texture = gl_LTexture_new();
  gl_LTexture_create_pixels8(combined_grayscale_texture, left_texture->width + right_texture->width, left_texture->height);

  // blit image
  gl_LTexture_blit_pixels8(left_texture, 0, 0, combined_grayscale_texture);
  gl_LTexture_blit_pixels8(right_texture, left_texture->width, 0, combined_grayscale_texture);

  // pad and create texture
  gl_LTexture_pad_pixels8(combined_grayscale_texture);
  gl_LTexture_load_texture_from_precreated_pixels8(combined_grayscale_texture);

  // get rid of old textures
  gl_LTexture_free(left_texture);
  gl_LTexture_free(right_texture);
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
  gl_LTexture_render(combined_texture, (g_logical_width - combined_texture->width) / 2.0f, 10.f, NULL);
  gl_LTexture_render(combined_grayscale_texture, (g_logical_width - combined_grayscale_texture->width) / 2.0f, combined_texture->height + 10.f, NULL);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{
  if (combined_texture != NULL)
    gl_LTexture_free(combined_texture);
  if (combined_grayscale_texture != NULL)
    gl_LTexture_free(combined_grayscale_texture);
}
