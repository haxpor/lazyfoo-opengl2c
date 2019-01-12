#include "gl_lfont_polygon_program2d.h"
#include "gl/gl_LShaderProgram.h"
#include "SDL_log.h"
#include <stdlib.h>

static void free_internals_(gl_lfont_polygon_program2d* program);

void free_internals_(gl_lfont_polygon_program2d* program)
{
  // reset all locations
  program->vertex_pos2d_location = -1;
  program->texture_coord_location = -1;
  program->projection_matrix_location = -1;
  program->modelview_matrix_location = -1;
  program->texture_sampler_location = -1;
  program->text_color_location = -1;

  // set matrix to identity
  glm_mat4_identity(program->projection_matrix);
  glm_mat4_identity(program->modelview_matrix);
  
  // free underlying shader program
  gl_LShaderProgram_free(program->program);
  program->program = NULL;
}

gl_lfont_polygon_program2d* gl_lfont_polygon_program2d_new()
{
  gl_lfont_polygon_program2d* out = malloc(sizeof(gl_lfont_polygon_program2d));

  // init defaults first
  out->program = NULL;
  out->vertex_pos2d_location = -1;
  out->texture_coord_location = -1;
  out->projection_matrix_location = -1;
  out->modelview_matrix_location = -1;
  out->texture_sampler_location = -1;
  out->text_color_location = -1;
  glm_mat4_identity(out->projection_matrix);
  glm_mat4_identity(out->modelview_matrix);

  // create underlying shader program
  // we will take care of this automatically when freeing
  gl_LShaderProgram* shader_program = gl_LShaderProgram_new();
  out->program = shader_program; 

  return out;
}

void gl_lfont_polygon_program2d_free(gl_lfont_polygon_program2d* program)
{
  // free internals
  free_internals_(program);

  // free source
  free(program);
  program = NULL;
}

bool gl_lfont_polygon_program2d_load_program(gl_lfont_polygon_program2d* program)
{
  // create a new program
  GLuint program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader_id = gl_LShaderProgram_load_shader_from_file("res/shaders/l_font_program2d.vert", GL_VERTEX_SHADER);
  if (vertex_shader_id == 0)
  {
    SDL_Log("Unable to load vertex shader from file");

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // attach vertex shader to shader program
  glAttachShader(program_id, vertex_shader_id);
  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error attaching vertex shader");
    gl_LShaderProgram_print_shader_log(vertex_shader_id);

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // load fragment shader
  GLuint fragment_shader_id = gl_LShaderProgram_load_shader_from_file("res/shaders/l_font_program2d.frag", GL_FRAGMENT_SHADER);
  if (fragment_shader_id == 0)
  {
    SDL_Log("Unable to load fragment shader from file");

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // attach fragment shader to program
  glAttachShader(program_id, fragment_shader_id);
  // check for errors
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error attaching fragment shader");
    gl_LShaderProgram_print_shader_log(fragment_shader_id);

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // link program
  glLinkProgram(program_id);
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error linking program");
    gl_LShaderProgram_print_program_log(program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader_id);
    fragment_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // set result program id to underlying program
  program->program->program_id = program_id;

  // mark shader for delete
  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);

  // get attribute locations
  program->vertex_pos2d_location = glGetAttribLocation(program_id, "vertex_pos2d");
  if (program->vertex_pos2d_location == -1)
  {
    SDL_Log("Warning: cannot get location of vertex_pos2d");
  }
  program->texture_coord_location = glGetAttribLocation(program_id, "texcoord");
  if (program->texture_coord_location == -1)
  {
    SDL_Log("Warning: cannot get location of texcoord");
  }

  // get uniform locations
  program->projection_matrix_location = glGetUniformLocation(program_id, "projection_matrix");
  if (program->projection_matrix_location == -1)
  {
    SDL_Log("Warning: cannot get location of projection_matrix");
  }
  program->modelview_matrix_location = glGetUniformLocation(program_id, "modelview_matrix");
  if (program->modelview_matrix_location == -1)
  {
    SDL_Log("Warning: cannot get location of modelview_matrix");
  }
  program->texture_sampler_location = glGetUniformLocation(program_id, "texture_sampler");
  if (program->texture_sampler_location == -1)
  {
    SDL_Log("Warning: cannot get location of texture_sampler");
  }
  program->text_color_location = glGetUniformLocation(program_id, "text_color");
  if (program->text_color_location == -1)
  {
    SDL_Log("Warning: cannot get location of text_color");
  }

  return true;
}

void gl_lfont_polygon_program2d_update_projection_matrix(gl_lfont_polygon_program2d* program)
{
  glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]);
}

void gl_lfont_polygon_program2d_update_modelview_matrix(gl_lfont_polygon_program2d* program)
{
  glUniformMatrix4fv(program->modelview_matrix_location, 1, GL_FALSE, program->modelview_matrix[0]);
}

void gl_lfont_polygon_program2d_set_vertex_pointer(gl_lfont_polygon_program2d* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void gl_lfont_polygon_program2d_set_texcoord_pointer(gl_lfont_polygon_program2d* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->texture_coord_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void gl_lfont_polygon_program2d_set_texture_sampler(gl_lfont_polygon_program2d* program, GLuint sampler)
{
  glUniform1i(program->texture_sampler_location, sampler);
}

void gl_lfont_polygon_program2d_set_text_color(gl_lfont_polygon_program2d* program, LColorRGBA color)
{
  glUniform4f(program->text_color_location, color.r, color.g, color.b, color.a);
}

void gl_lfont_polygon_program2d_enable_attrib_pointers(gl_lfont_polygon_program2d* program)
{
  glEnableVertexAttribArray(program->vertex_pos2d_location);
  glEnableVertexAttribArray(program->texture_coord_location); 
}

void gl_lfont_polygon_program2d_disable_attrib_pointers(gl_lfont_polygon_program2d* program)
{
  glDisableVertexAttribArray(program->vertex_pos2d_location);
  glDisableVertexAttribArray(program->texture_coord_location);
}
