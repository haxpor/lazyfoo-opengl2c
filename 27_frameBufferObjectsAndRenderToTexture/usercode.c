#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "gl/gl_util.h"
#include "gl/gl_LTexture.h"
#include "gl/gl_LTexture_internals.h"

#define RENDER_DECORATION_BACKGROUND_WIDESCREEN 0

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
static gl_LTexture *opengl_texture = NULL;
static GLfloat texture_angle = 0.f;

// frame buffer
GLuint fbo = 0;
// texture that fbo will render to
gl_LTexture* fbo_texture = NULL;

static void render_scene();

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
  // enable blending with default blend function
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
  // as we asked for opengl 2.1
  // we initially check whether the system supports framebuffer object, if not then return now
  // note: actually check here is a little bit late, we could do it in usercode_init(), but here works as well
  // check the name from extension registry, for fbo it's https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_object.txt
  if (!GLEW_ARB_framebuffer_object)
  {
    SDL_Log("system doesn't supported framebuffer object");
    return false;
  }

  // generate framebuffer name
  glGenFramebuffers(1, &fbo);
  // check for error related to opengl
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error generating frame buffers %s", gl_util_error_string(error));
    return false;
  }

  // allocate fbo texture
  fbo_texture = gl_LTexture_new();

  // load texture
  opengl_texture = gl_LTexture_new();
  if (!gl_LTexture_load_texture_from_file(opengl_texture, "opengl.png"))
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

void take_snapshot_then_render_into_fbotexture()
{
  // bind framebuffer for use
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // if fbo texture doesn't exist
  if (fbo_texture->texture_id == 0)
  {
    // create it
    gl_LTexture_create_pixels32(fbo_texture, g_logical_width, g_logical_height);
    gl_LTexture_pad_pixels32(fbo_texture);
    gl_LTexture_load_texture_from_precreated_pixels32(fbo_texture);
  }

  // bind texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture->texture_id, 0);

  // clear frame buffer
  glClear(GL_COLOR_BUFFER_BIT);

  // check if it's in full-screen, then we need to set viewport properly
  int viewport_setups[4];
  if (gWindow->fullscreen)
  {
    // save current states of viewport's setups first
    glGetIntegerv(GL_VIEWPORT, viewport_setups);
    // set viewport to get rid of offset x, y as it's in local coordinate of texture now; no offset for wide-screen
    glViewport(0, 0, g_logical_width, g_logical_height);
  }

  // render scenes
  render_scene();

  if (gWindow->fullscreen)
  {
    // set viewport back to previous setups
    glViewport(viewport_setups[0], viewport_setups[1], viewport_setups[2], viewport_setups[3]);
  }

  // unbind framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
      take_snapshot_then_render_into_fbotexture();
    }
  }
}

void usercode_update(float delta_time)
{
  // update rotations
  texture_angle += 6 % 360;
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
#if RENDER_DECORATION_BACKGROUND_WIDESCREEN
    // note: we can do this better for performance by caching viewport's setup values as well as window's dimensions
    // so we have no need to query it every frame
    // get current set up of viewport as we will render over full screen, not just content area
    // only when it need clipping (or went fullscreen), that's why this code is inside here
    int viewport_setups[4];
    glGetIntegerv(GL_VIEWPORT, viewport_setups);
    // set viewport properly for full screen (no scale) of rendering area
    glViewport(0, 0, g_screen_width, g_screen_height);

    // render background when go full screen
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBindTexture(GL_TEXTURE_2D, 0);
    glTranslatef(g_ri_view_width/2.f, g_ri_view_height/2.f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glBegin(GL_QUADS);
      glVertex2f(-g_ri_view_width/2.f, -g_ri_view_height/2.f);
      glVertex2f(g_ri_view_width/2.f, -g_ri_view_height/2.f);
      glVertex2f(g_ri_view_width/2.f, g_ri_view_height/2.f);
      glVertex2f(-g_ri_view_width/2.f, g_ri_view_height/2.f);
    glEnd();
    glPopMatrix();
    
    // set back to original viewport
    glViewport(viewport_setups[0], viewport_setups[1], viewport_setups[2], viewport_setups[3]);
#endif
    
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
  if (fbo_texture->texture_id != 0)
  {
    glPushMatrix();
    glTranslatef((g_logical_width - fbo_texture->width)/ 2.f, (g_logical_height - fbo_texture->height) / 2.f, 0.f);
    glTranslatef(fbo_texture->width / 2.f, fbo_texture->height / 2.f, 0.f);
    glRotatef(-texture_angle/2.f, 0.f, 0.f, 1.f);
    glTranslatef(-fbo_texture->width / 2.f, -fbo_texture->height / 2.f, 0.f);
    glColor3f(1.f, 1.f, 1.f);
    gl_LTexture_render(fbo_texture, 0.f, 0.f, NULL);
    glPopMatrix();
  }

  // render scene
  render_scene();

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void render_scene()
{
  // make sure we use modelview matrix mode
  glMatrixMode(GL_MODELVIEW);

  // render texture
  // note: rotate texture around its center via opengl's matrix transformations
  glPushMatrix();
  glTranslatef((g_logical_width - opengl_texture->width) / 2.f, (g_logical_height - opengl_texture->height) / 2.f, 0.f);
  glTranslatef(opengl_texture->width / 2.f, opengl_texture->height / 2.f, 0.f);
  glRotatef(texture_angle, 0.f, 0.f, 1.f);
  glTranslatef(-opengl_texture->width / 2.f, -opengl_texture->height / 2.f, 0.f);
  glColor3f(1.f, 1.f, 1.f);
  // set position to NULL as we've transformed matrix it via opengl
  gl_LTexture_render(opengl_texture, 0.f, 0.f, NULL);
  glPopMatrix();

  // unbind texture for geometry
  glBindTexture(GL_TEXTURE_2D, 0);

  // red quad
  // note: rotate quad around its center, no need to translate band and forth around glRotatef() as
  // quad's vertices positions are defined around the center position of the quad itself
  glPushMatrix();
  glTranslatef(g_logical_width * 1.f / 4.f, g_logical_height * 1.f / 4.f, 0.f);
  glRotatef(texture_angle, 0.f, 0.f, 1.f);
  glColor3f(1.f, 0.f, 0.f);
  glBegin(GL_QUADS);
    glVertex2f(-g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, g_logical_height / 16.f);
    glVertex2f(-g_logical_width / 16.f, g_logical_height / 16.f);
  glEnd();
  glPopMatrix();

  // green quad
  glPushMatrix();
  glTranslatef(g_logical_width * 3.f / 4.f, g_logical_height * 1.f / 4.f, 0.f);
  glRotatef(texture_angle, 0.f, 0.f, 1.f);
  glColor3f(0.f, 1.f, 0.f);
  glBegin(GL_QUADS);
    glVertex2f(-g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, g_logical_height / 16.f);
    glVertex2f(-g_logical_width / 16.f, g_logical_height / 16.f);
  glEnd();
  glPopMatrix();

  // blue quad
  glPushMatrix();
  glTranslatef(g_logical_width * 1.f / 4.f, g_logical_height * 3.f / 4.f, 0.f);
  glRotatef(texture_angle, 0.f, 0.f, 1.f);
  glColor3f(0.f, 0.f, 1.f);
  glBegin(GL_QUADS);
    glVertex2f(-g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, g_logical_height / 16.f);
    glVertex2f(-g_logical_width / 16.f, g_logical_height / 16.f);
  glEnd();
  glPopMatrix();

  // yellow quad
  glPushMatrix();
  glTranslatef(g_logical_width * 3.f / 4.f, g_logical_height * 3.f / 4.f, 0.f);
  glRotatef(texture_angle, 0.f, 0.f, 1.f);
  glColor3f(1.f, 1.f, 0.f);
  glBegin(GL_QUADS);
    glVertex2f(-g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, -g_logical_height / 16.f);
    glVertex2f(g_logical_width / 16.f, g_logical_height / 16.f);
    glVertex2f(-g_logical_width / 16.f, g_logical_height / 16.f);
  glEnd();
  glPopMatrix();
}

void usercode_close()
{
  if (opengl_texture != NULL)
    gl_LTexture_free(opengl_texture);
  if (fbo_texture != NULL)
    gl_LTexture_free(fbo_texture);
  // delete fbo buffer
  glDeleteBuffers(1, &fbo);
  fbo = 0;
}
