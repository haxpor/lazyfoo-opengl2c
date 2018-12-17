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

static bool g_need_clipping = false;

// camera position
GLfloat g_camera_x = 0.f;
GLfloat g_camera_y = 0.f;

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

  // save modelview matrix
  glPushMatrix();

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

void usercode_set_screen_dimension(Uint32 window_id, int screen_width, int screen_height)
{
  g_screen_width = screen_width;
  g_screen_height = screen_height;
}

void usercode_handle_event(SDL_Event *e, float delta_time)
{
  bool cam_needupdate = false;

  // get pointer to key states as we need to check/operate while the key is being pressed
  const Uint8* key_states = SDL_GetKeyboardState(NULL);
  
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
    
    if (k == SDLK_w || key_states[SDL_SCANCODE_W] == 1)
    {
      g_camera_y -= 16.f;
      cam_needupdate = true;
    }
    if (k == SDLK_s || key_states[SDL_SCANCODE_S] == 1)
    {
      g_camera_y += 16.f;
      cam_needupdate = true;
    }
    if (k == SDLK_a || key_states[SDL_SCANCODE_A] == 1)
    {
      g_camera_x -= 16.f;
      cam_needupdate = true;
    }
    if (k == SDLK_d || key_states[SDL_SCANCODE_D] == 1)
    {
      g_camera_x += 16.f;
      cam_needupdate = true;
    }
  }

  if (cam_needupdate)
  {
    // take saved matrix off the stack and reset it
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glLoadIdentity();

    // move camera to position
    glTranslatef(-g_camera_x, -g_camera_y, 0.f);

    // save default matrix again with camera translation
    glPushMatrix();
  }
}

void usercode_update(float delta_time)
{

}

void usercode_render()
{
  // clear color buffer
  if (g_need_clipping)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
  // pop default matrix onto the current one
  glPopMatrix();
  // immediately push matrix to stack as we need to use it now
  glPushMatrix();
  
  // move to center of the screen
  glTranslatef(g_ri_view_width / 2.f, g_ri_view_height / 2.f, 0.f);
  // set scale
  glScalef(g_ri_scale_x, g_ri_scale_y, 1.0f);

  // red quad
  glBegin(GL_QUADS);
    glColor3f(1.f, 0.f, 0.f);
    glVertex2f(-g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f,  g_logical_height / 4.f);
    glVertex2f(-g_logical_width / 4.f,  g_logical_height / 4.f);
  glEnd();

  // move to the right of the screen
  glTranslatef(g_logical_width, 0.f, 0.f);
  // green quad
  glBegin(GL_QUADS);
    glColor3f(0.f, 1.f, 0.f);
    glVertex2f(-g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f,  g_logical_height / 4.f);
    glVertex2f(-g_logical_width / 4.f,  g_logical_height / 4.f);
  glEnd();

  // move to lower right of the screen (due to effect of previous moving to the right matrix)
  glTranslatef(0.f, g_logical_width, 0.f);
  // blue quad
  glBegin(GL_QUADS);
    glColor3f(0.f, 0.f, 1.f);
    glVertex2f(-g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f,  g_logical_height / 4.f);
    glVertex2f(-g_logical_width / 4.f,  g_logical_height / 4.f);
  glEnd();

  // move below screen (remove effect of translating to the right from red quad)
  glTranslatef(-g_logical_width, 0.f, 0.f);
  // yellow quad
  glBegin(GL_QUADS);
    glColor3f(1.f, 1.f, 0.f);
    glVertex2f(-g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f, -g_logical_height / 4.f);
    glVertex2f( g_logical_width / 4.f,  g_logical_height / 4.f);
    glVertex2f(-g_logical_width / 4.f,  g_logical_height / 4.f);
  glEnd();

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{

}
