#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "gl/gl_util.h"
#include "gl/gl_LTexture.h"
#include "gl/gl_LShaderProgram.h"
#include "gl/gl_ltextured_polygon_program2d.h"

// don't use this elsewhere
#define CONTENT_BG_COLOR 1.f, 0.f, 0.f, 1.f

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

static mat4 g_projection_matrix;
// base modelview matrix to reduce some of mathematics operation initially
static mat4 g_base_modelview_matrix;
// -- section of variables for maintaining aspect ratio -- //

// -- section of function signatures -- //
static void usercode_app_went_windowed_mode();
static void usercode_app_went_fullscreen();
// -- end of section of function signatures -- //

static gl_ltextured_polygon_program2d* textured_shader = NULL;
// vao
GLuint vao = 0;
// loaded texture
static gl_LTexture* opengl_texture = NULL;
static LColorRGBA opengl_texture_color = {1.f, 1.f, 1.f, 0.75f};

void usercode_app_went_windowed_mode()
{
  // copy calculated projection matrix to shader's then update to shader
	glm_mat4_copy(g_projection_matrix, textured_shader->projection_matrix);

	// copy calculated modelview matrix to shader's
	glm_mat4_copy(g_base_modelview_matrix, textured_shader->modelview_matrix);

  // bind shader to initially update projection matrix to GPU
  gl_LShaderProgram_bind(textured_shader->program);
    // update projection & modelview matrix
    gl_ltextured_polygon_program2d_update_projection_matrix(textured_shader);
  // unbind program
  gl_LShaderProgram_unbind(textured_shader->program);
}

void usercode_app_went_fullscreen()
{
  // copy calculated projection matrix to shader's then update to shader
	glm_mat4_copy(g_projection_matrix, textured_shader->projection_matrix);

	// copy calculated modelview matrix to shader's
	glm_mat4_copy(g_base_modelview_matrix, textured_shader->modelview_matrix);

  // bind shader to initially update projection matrix to GPU
  gl_LShaderProgram_bind(textured_shader->program);
    // update projection & modelview matrix
    gl_ltextured_polygon_program2d_update_projection_matrix(textured_shader);
  // unbind program
  gl_LShaderProgram_unbind(textured_shader->program);
}

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

  // calculate orthographic projection matrix
	glm_ortho(0.0, g_screen_width, g_screen_height, 0.0, -1.0, 1.0, g_projection_matrix);
	// calculate base modelview matrix (to reduce some of operations cost)
	glm_mat4_identity(g_base_modelview_matrix);
	glm_scale(g_base_modelview_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

  // initialize the viewport
  // define the area where to render, for now full screen
  glViewport(0, 0, g_screen_width, g_screen_height);

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error initializing OpenGL! %s", gl_util_error_string(error));
    return false;
  }

  return true;
}

bool usercode_loadmedia()
{
  // TODO: Load media here...
  // vao
  glGenVertexArrays(1, &vao);

  textured_shader = gl_ltextured_polygon_program2d_new();
  if (!gl_ltextured_polygon_program2d_load_program(textured_shader))
  {
    return false;
  }

  // initially set projection & modelview matrix to shader
  glm_mat4_copy(g_projection_matrix, textured_shader->projection_matrix);
  glm_mat4_copy(g_base_modelview_matrix, textured_shader->modelview_matrix);
  
  // set textured shader to gl_LTexture
  gl_LTexture_set_shared_shader(textured_shader);

  // bind shader to initially update stuff
  gl_LShaderProgram_bind(textured_shader->program);
  // set texture sampler
  gl_ltextured_polygon_program2d_set_texture_sampler(textured_shader, 0);
  // update projection & modelview matrix
  gl_ltextured_polygon_program2d_update_projection_matrix(textured_shader);
  // unbind program
  gl_LShaderProgram_unbind(textured_shader->program);

  // load opengl texture
  opengl_texture = gl_LTexture_new();
  if (!gl_LTexture_load_texture_from_file(opengl_texture, "opengl.png"))
  {
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
				
				// re-calculate orthographic projection matrix
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -1.0, 1.0, g_projection_matrix);

				// re-calculate base modelview matrix
				// no need to scale as it's uniform 1.0 now
				glm_mat4_identity(g_base_modelview_matrix);

				// signal that app went windowed mode
				usercode_app_went_windowed_mode();
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

				// re-calculate orthographic projection matrix
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -1.0, 1.0, g_projection_matrix);

				// re-calculate base modelview matrix
				glm_mat4_identity(g_base_modelview_matrix);
				// also scale
				glm_scale(g_base_modelview_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

				// signal that app went fullscreen mode
				usercode_app_went_fullscreen();
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
    glClearColor(CONTENT_BG_COLOR);
  glClear(GL_COLOR_BUFFER_BIT);

  // now clip content to be drawn only on content area (if needed)
  if (g_need_clipping)
  {
    // clear color for content area
    glEnable(GL_SCISSOR_TEST);
    glScissor(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
    glClearColor(CONTENT_BG_COLOR);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  // note: set viewport via glViewport accordingly, you should start at g_offset_x, and g_offset_y
  // note2: glViewport coordinate still in world coordinate, but for individual object (vertices) to be drawn, it's local coordinate

  // TODO: render code goes here...
  // bind vao
  glBindVertexArray(vao);
  
  // bind program
  gl_LShaderProgram_bind(textured_shader->program);
  
  // start modelview from base
  // modelview matrix will be updated inside function call
  glm_mat4_copy(g_base_modelview_matrix, textured_shader->modelview_matrix);
  // set texture color
  gl_ltextured_polygon_program2d_set_texture_color(textured_shader, opengl_texture_color);
  // render texture
  gl_LTexture_render(opengl_texture, (g_logical_width - opengl_texture->width)/2.f, (g_logical_height - opengl_texture->height)/2.f, NULL);

  // unbind program
  gl_LShaderProgram_unbind(textured_shader->program);

  // unbind vao
  glBindVertexArray(0);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{
  if (textured_shader != NULL)
    gl_ltextured_polygon_program2d_free(textured_shader);

  if (opengl_texture != NULL)
    gl_LTexture_free(opengl_texture);

  if (vao != 0)
  {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
}
