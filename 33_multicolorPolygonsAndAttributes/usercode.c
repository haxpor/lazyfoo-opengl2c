#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "gl/gl_util.h"
#include "gl/gl_LTexture.h"
#include "gl/gl_LShaderProgram.h"
#include "gl/gl_LMultiColorPolygonProgram2D.h"
#include <stddef.h>

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

// -- end section of variables for maintaining aspect ratio -- //
// -- section of function signatures -- //
static void usercode_app_went_windowed_mode();
static void usercode_app_went_fullscreen();
// -- end of section of function signatures --//
static gl_LMultiColorPolygonProgram2D* multi_color_shader = NULL;
static GLuint vao = 0;
static GLuint vbo = 0;
static GLuint ibo = 0;

void usercode_app_went_windowed_mode()
{
	// copy calculated projection matrix to shader's then update to shader
	glm_mat4_copy(g_projection_matrix, multi_color_shader->projection_matrix);

	// copy calculated modelview matrix to shader's
	glm_mat4_copy(g_base_modelview_matrix, multi_color_shader->modelview_matrix);
}

void usercode_app_went_fullscreen()
{
	// copy calculated projection matrix to shader's then update to shader
	glm_mat4_copy(g_projection_matrix, multi_color_shader->projection_matrix);

	// copy calculated modelview matrix to shader's
	glm_mat4_copy(g_base_modelview_matrix, multi_color_shader->modelview_matrix);
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

  // enable texturing
  glEnable(GL_TEXTURE_2D);
  // enable blending with default blend function
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // check for errors
  // FIXME: Unexpected error from opengl init stuff, but ignore it still ok for us to see the result. Not sure how to fix this for now.
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Warning initializing OpenGL! %s", gl_util_error_string(error));
  }

  return true;
}

bool usercode_loadmedia()
{
  // TODO: Load media here...

  // init shader
  gl_LShaderProgram* raw_shader = gl_LShaderProgram_new();
  multi_color_shader = gl_LMultiColorPolygonProgram2D_new(raw_shader);
  if (!gl_LMultiColorPolygonProgram2D_load_program(multi_color_shader))
  {
    return false;
  }

  // vao
  glGenVertexArrays(1, &vao);

  // vbo data
  LMultiColorVertex2D quad_vertices[4];
  GLuint indices[4];

  // set quad vertices
  quad_vertices[0].pos.x = -50.f;
  quad_vertices[0].pos.y = -50.f;
  quad_vertices[0].color.r = 1.f;
  quad_vertices[0].color.g = 0.f;
  quad_vertices[0].color.b = 0.f;
  quad_vertices[0].color.a = 1.f;

  quad_vertices[1].pos.x = 50.f;
  quad_vertices[1].pos.y = -50.f;
  quad_vertices[1].color.r = 1.f;
  quad_vertices[1].color.g = 1.f;
  quad_vertices[1].color.b = 0.f;
  quad_vertices[1].color.a = 1.f;

  quad_vertices[2].pos.x = 50.f;
  quad_vertices[2].pos.y = 50.f;
  quad_vertices[2].color.r = 0.f;
  quad_vertices[2].color.g = 1.f;
  quad_vertices[2].color.b = 0.f;
  quad_vertices[2].color.a = 1.f;

  quad_vertices[3].pos.x = -50.f;
  quad_vertices[3].pos.y = 50.f;
  quad_vertices[3].color.r = 0.f;
  quad_vertices[3].color.g = 0.f;
  quad_vertices[3].color.b = 1.f;
  quad_vertices[3].color.a = 1.f;

  // set rendering indices
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;
  indices[3] = 3;

  // create vbo
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(LMultiColorVertex2D), quad_vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indices, GL_STATIC_DRAW);

  // initially calculate projection and modelview matrix
  glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -1.0, 1.0, g_projection_matrix);
  glm_mat4_copy(g_projection_matrix, multi_color_shader->projection_matrix);

  glm_mat4_identity(g_base_modelview_matrix);
  glm_mat4_copy(g_base_modelview_matrix, multi_color_shader->modelview_matrix);

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

  // note: set viewport via glViewport accordingly, you should start at g_offset_x, and g_offset_y
  // note2: glViewport coordinate still in world coordinate, but for individual object (vertices) to be drawn, it's local coordinate

  // TODO: render code goes here...

  // bind program
  gl_LShaderProgram_bind(multi_color_shader->program);

  // bind vao
  glBindVertexArray(vao);

  // update projection matrix
  gl_LMultiColorPolygonProgram2D_update_projection_matrix(multi_color_shader);

  // update modelview matrix
  glm_mat4_copy(g_base_modelview_matrix, multi_color_shader->modelview_matrix);
  glm_translate(multi_color_shader->modelview_matrix, (vec3){g_logical_width / 2.f, g_logical_height / 2.f, 0.f});
  gl_LMultiColorPolygonProgram2D_update_modelview_matrix(multi_color_shader);

  // enable all generic vertex attributes
  gl_LMultiColorPolygonProgram2D_enable_attrib_pointers(multi_color_shader);

  // set vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  gl_LMultiColorPolygonProgram2D_set_vertex_pointer(multi_color_shader, sizeof(LMultiColorVertex2D), (const GLvoid*)offsetof(LMultiColorVertex2D, pos));
  gl_LMultiColorPolygonProgram2D_set_color_pointer(multi_color_shader, sizeof(LMultiColorVertex2D), (const GLvoid*)offsetof(LMultiColorVertex2D, color));

  // render quad using vertex dat and index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

  // disable all generic vertex attributes
  gl_LMultiColorPolygonProgram2D_disable_attrib_pointers(multi_color_shader);

  // unbind vao
  glBindVertexArray(0);

  // unbind program
  gl_LShaderProgram_unbind(multi_color_shader->program);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_close()
{
  if (multi_color_shader != NULL)
    gl_LMultiColorPolygonProgram2D_free(multi_color_shader);

  if (vbo != 0)
  {
    glDeleteBuffers(1, &vbo);
    vbo = 0;
  }

  if (ibo != 0)
  {
    glDeleteBuffers(1, &ibo);
    ibo = 0;
  }

  if (vao != 0)
  {
    glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
}
