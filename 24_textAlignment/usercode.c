#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "gl/gl_util.h"
#include "gl/gl_LTexture.h"
#include "gl/gl_LTexture_spritesheet.h"
#include "gl/gl_LFont.h"

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
static gl_LFont* ttf_font = NULL;

// text alignments
static enum gl_LFont_TextAlignment text_h_align = gl_LFont_TEXT_ALIGN_LEFT;
static enum gl_LFont_TextAlignment text_v_align = gl_LFont_TEXT_ALIGN_TOP;
static int text_align; // will be filled at load time
// text area size
static LSize text_area; // to be filled at load time

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
  text_area.w = g_logical_width;
  text_area.h = g_logical_height;

  text_align = (int)text_h_align | (int)text_v_align;

  gl_LTexture* raw_texture = gl_LTexture_new();
  gl_LSpritesheet* spritesheet = gl_LSpritesheet_new(raw_texture);
  ttf_font = gl_LFont_new(spritesheet);
  
  if (!gl_LFont_load_freetype(ttf_font, "lazy.ttf", 60))
  {
    SDL_Log("Unable to create ttf font");
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
    // cycle backward for horizontal alignmeent
    else if (k == SDLK_a)
    {
      if (text_h_align == gl_LFont_TEXT_ALIGN_LEFT)
      {
        text_h_align = gl_LFont_TEXT_ALIGN_RIGHT;
      }
      else if (text_h_align == gl_LFont_TEXT_ALIGN_CENTERED_H)
      {
        text_h_align = gl_LFont_TEXT_ALIGN_LEFT;
      }
      else if (text_h_align == gl_LFont_TEXT_ALIGN_RIGHT)
      {
        text_h_align = gl_LFont_TEXT_ALIGN_CENTERED_H;
      }

      // recalculate text_align
      text_align = (int)text_h_align | (int)text_v_align;
    }
    // cycle forward for horizontal alignment
    else if (k == SDLK_d)
    {
      if (text_h_align == gl_LFont_TEXT_ALIGN_LEFT)
      {
        text_h_align = gl_LFont_TEXT_ALIGN_CENTERED_H;
      }
      else if (text_h_align == gl_LFont_TEXT_ALIGN_CENTERED_H)
      {
        text_h_align = gl_LFont_TEXT_ALIGN_RIGHT;
      }
      else if (text_h_align == gl_LFont_TEXT_ALIGN_RIGHT)
      {
        text_h_align = gl_LFont_TEXT_ALIGN_LEFT;
      }

      // recalculate text_align
      text_align = (int)text_h_align | (int)text_v_align;
    }
    // cycle backward for vertical alignmeent
    else if (k == SDLK_w)
    {
      if (text_v_align == gl_LFont_TEXT_ALIGN_TOP)
      {
        text_v_align = gl_LFont_TEXT_ALIGN_BOTTOM;
      }
      else if (text_v_align == gl_LFont_TEXT_ALIGN_CENTERED_V)
      {
        text_v_align = gl_LFont_TEXT_ALIGN_TOP;
      }
      else if (text_v_align == gl_LFont_TEXT_ALIGN_BOTTOM)
      {
        text_v_align = gl_LFont_TEXT_ALIGN_CENTERED_V;
      }

      // recalculate text_align
      text_align = (int)text_h_align | (int)text_v_align;
    }
    // cycle forward for vertical alignment
    else if (k == SDLK_s)
    {
      if (text_v_align == gl_LFont_TEXT_ALIGN_TOP)
      {
        text_v_align = gl_LFont_TEXT_ALIGN_CENTERED_V;
      }
      else if (text_v_align == gl_LFont_TEXT_ALIGN_CENTERED_V)
      {
        text_v_align = gl_LFont_TEXT_ALIGN_BOTTOM;
      }
      else if (text_v_align == gl_LFont_TEXT_ALIGN_BOTTOM)
      {
        text_v_align = gl_LFont_TEXT_ALIGN_TOP;
      }

      // recalculate text_align
      text_align = (int)text_h_align | (int)text_v_align;
    }
    else if (k == SDLK_1)
    {
      if (!gl_LFont_load_freetype(ttf_font, "lazy.ttf", 60))
      {
        SDL_Log("Unable to create ttf font - lazy");
      }
    }
    else if (k == SDLK_2)
    {
      if (!gl_LFont_load_freetype(ttf_font, "Minecraft.ttf", 40))
      {
        SDL_Log("Unable to create ttf font - Minecraft");
      }
    }
    else if (k == SDLK_3)
    {
      if (!gl_LFont_load_freetype(ttf_font, "8bitwonder.ttf", 40))
      {
        SDL_Log("Unable to create ttf font - 8bitwonder");
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
  glColor3f(1.f, 1.f, 0.f);
  gl_LFont_render_textex(ttf_font, "Testing\nAlignment...", 0.f, 0.0f, &text_area, text_align);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{
  if (ttf_font != NULL)
    gl_LFont_free(ttf_font);
}
