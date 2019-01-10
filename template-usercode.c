#include "usercode.h"
#include "foundation/common.h"
#include "foundation/LWindow.h"
#include "foundation/krr_util.h"
#include "gl/gl_util.h"
#include "gl/gl_LTexture.h"
#include "gl/gl_ltextured_polygon_program2d.h"
#include "gl/gl_LFont.h"
#include "gl/gl_lfont_polygon_program2d.h"

// don't use this elsewhere
#define CONTENT_BG_COLOR 1.f, 0.f, 0.f, 1.f

#ifndef DISABLE_FPS_CALC
#define FPS_BUFFER 7+1
char fps_text[FPS_BUFFER];
static gl_LFont* fps_font = NULL;
#endif

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

enum usercode_matrixtype
{
  usercode_matrixtype_projection_matrix,
  usercode_matrixtype_modelview_matrix
};
enum usercode_shadertype
{
  usercode_shadertype_texture_shader,
  usercode_shadertype_font_shader
};
///
/// set matrix then update to shader
/// required user to bind the shader before calling this function.
///
/// \param matrix_type type of matrix to copy to dst. Value is enum usercode_matrixtype.
/// \param shader_type type of shader. Value is enum usercode_shadertype.
/// \param program pointer to shader program.
///
static void usercode_set_matrix_then_update_to_shader(enum usercode_matrixtype matrix_type, enum usercode_shadertype shader_type, void* program);
// -- end of section of function signatures -- //

#ifndef DISABLE_FPS_CALC
static GLuint fps_vao = 0;
#endif

// basic shaders and font
static gl_ltextured_polygon_program2d* texture_shader = NULL;
static gl_lfont_polygon_program2d* font_shader = NULL;
static gl_LFont* font = NULL;

void usercode_set_matrix_then_update_to_shader(enum usercode_matrixtype matrix_type, enum usercode_shadertype shader_program, void* program)
{
  // projection matrix
  if (matrix_type == usercode_matrixtype_projection_matrix)
  {
    // texture shader
    if (shader_program == usercode_shadertype_texture_shader)
    {
      // convert to right type of program shader
      gl_ltextured_polygon_program2d* shader_ptr = (gl_ltextured_polygon_program2d*)program;

      // copy calculated projection matrix to shader's then update to shader
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);

      gl_ltextured_polygon_program2d_update_projection_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == usercode_shadertype_font_shader)
    {
      gl_lfont_polygon_program2d* shader_ptr = (gl_lfont_polygon_program2d*)program;
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);

      gl_lfont_polygon_program2d_update_projection_matrix(shader_ptr);
    }
  }
  // modelview matrix
  else if (matrix_type == usercode_matrixtype_modelview_matrix)
  {
    // texture shader
    if (shader_program == usercode_shadertype_texture_shader)
    {
      gl_ltextured_polygon_program2d* shader_ptr = (gl_ltextured_polygon_program2d*)program;
      glm_mat4_copy(g_base_modelview_matrix, shader_ptr->modelview_matrix);

      gl_ltextured_polygon_program2d_update_modelview_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == usercode_shadertype_font_shader)
    {
      gl_lfont_polygon_program2d* shader_ptr = (gl_lfont_polygon_program2d*)program;
      glm_mat4_copy(g_base_modelview_matrix, shader_ptr->modelview_matrix);

      gl_lfont_polygon_program2d_update_modelview_matrix(shader_ptr);
    }
  }
}

void usercode_app_went_windowed_mode()
{
  gl_LShaderProgram_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_projection_matrix, usercode_shadertype_texture_shader, texture_shader);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_modelview_matrix, usercode_shadertype_texture_shader, texture_shader);
  // no need to unbind as we will bind a new one soon

  gl_LShaderProgram_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_projection_matrix, usercode_shadertype_font_shader, font_shader);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_modelview_matrix, usercode_shadertype_font_shader, font_shader);
  gl_LShaderProgram_unbind(font_shader->program);
}

void usercode_app_went_fullscreen()
{
  gl_LShaderProgram_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_projection_matrix, usercode_shadertype_texture_shader, texture_shader);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_modelview_matrix, usercode_shadertype_texture_shader, texture_shader);

  gl_LShaderProgram_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_projection_matrix, usercode_shadertype_font_shader, font_shader);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_modelview_matrix, usercode_shadertype_font_shader, font_shader);
  gl_LShaderProgram_unbind(font_shader->program);
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

  // enable blending with default blend function
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    krr_util_print_callstack();
    SDL_Log("Error initializing OpenGL! %s", gl_util_error_string(error));
    return false;
  }

  return true;
}

bool usercode_loadmedia()
{
  // TODO: Load media here...
  // load font to render framerate
#ifndef DISABLE_FPS_CALC
  {
    glGenVertexArrays(1, &fps_vao);

    gl_LTexture* raw_texture = gl_LTexture_new();
    gl_LSpritesheet* spritesheet = gl_LSpritesheet_new(raw_texture);

    fps_font = gl_LFont_new(spritesheet);
    if (!gl_LFont_load_freetype(fps_font, "../Minecraft.ttf", 14))
    {
      SDL_Log("Unable to load font for rendering framerate");
      return false;
    }
  }
#endif
  
  // create font
  gl_LTexture* raw_texture = gl_LTexture_new();
  gl_LSpritesheet* ss = gl_LSpritesheet_new(raw_texture);
  font = gl_LFont_new(ss);
  if (!gl_LFont_load_freetype(font, "../Minecraft.ttf", 40))
  {
    SDL_Log("Error to load font");
    return false;
  }
  // load texture shader
  texture_shader = gl_ltextured_polygon_program2d_new();
  if (!gl_ltextured_polygon_program2d_load_program(texture_shader))
  {
    SDL_Log("Error loading texture shader");
    return false;
  }
  
  // load font shader
  font_shader = gl_lfont_polygon_program2d_new();
  if (!gl_lfont_polygon_program2d_load_program(font_shader))
  {
    SDL_Log("Error loading font shader");
    return false;
  }

  // initially update all related matrices and related graphics stuf for both shaders
  gl_LShaderProgram_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_projection_matrix, usercode_shadertype_texture_shader, texture_shader);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_modelview_matrix, usercode_shadertype_texture_shader, texture_shader);
  gl_ltextured_polygon_program2d_set_texture_sampler(texture_shader, 0);
  // set texture shader to all gl_LTexture as active
  shared_textured_shaderprogram = texture_shader;

  gl_LShaderProgram_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_projection_matrix, usercode_shadertype_font_shader, font_shader);
  usercode_set_matrix_then_update_to_shader(usercode_matrixtype_modelview_matrix, usercode_shadertype_font_shader, font_shader);
  gl_lfont_polygon_program2d_set_texture_sampler(font_shader, 0);
  // set font shader to all gl_LFont as active
  shared_font_shaderprogram = font_shader;
  gl_LShaderProgram_unbind(font_shader->program);
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

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_render_fps(int avg_fps)
{
#ifndef DISABLE_FPS_CALC
  // form framerate string to render
  snprintf(fps_text, FPS_BUFFER-1, "%d", avg_fps);

  // bind fps-vao
  glBindVertexArray(fps_vao);

  // use shared font shader
  gl_LShaderProgram_bind(shared_font_shaderprogram->program);
    // start with clean state of modelview matrix
    glm_mat4_copy(g_base_modelview_matrix, shared_font_shaderprogram->modelview_matrix);
    gl_lfont_polygon_program2d_update_modelview_matrix(shared_font_shaderprogram);

    // render text on top right
    gl_LFont_render_textex(fps_font, fps_text, 0.f, 4.f, &(LSize){g_logical_width, g_logical_height}, gl_LFont_TEXT_ALIGN_RIGHT | gl_LFont_TEXT_ALIGN_TOP);
  gl_LShaderProgram_unbind(shared_font_shaderprogram->program);

  // unbind fps-vao
  glBindVertexArray(0);
#endif 
}

void usercode_close()
{
#ifndef DISABLE_FPS_CALC
  if (fps_vao == 0)
  {
    glDeleteVertexArrays(1, &fps_vao);
    fps_vao = 0;
  }
  if (fps_font != NULL)
    gl_LFont_free(fps_font);
#endif
  if (font != NULL)
    gl_LFont_free(font);
  if (font_shader != NULL)
    gl_lfont_polygon_program2d_free(font_shader);
  if (texture_shader != NULL)
    gl_ltextured_polygon_program2d_free(texture_shader);
}
