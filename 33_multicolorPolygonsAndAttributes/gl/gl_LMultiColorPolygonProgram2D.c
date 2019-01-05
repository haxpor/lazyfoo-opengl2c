#include "gl_LMultiColorPolygonProgram2D.h"
#include <stdlib.h>
#include "SDL_log.h"

static void init_defaults(gl_LMultiColorPolygonProgram2D* program);

void init_defaults(gl_LMultiColorPolygonProgram2D* program)
{
  program->program = NULL;
  program->vertex_pos2d_location = -1;
  program->multi_color_location = -1;
  glm_mat4_identity(program->projection_matrix);
  program->projection_matrix_location = -1;
  glm_mat4_identity(program->modelview_matrix);
  program->modelview_matrix_location = -1;
}

gl_LMultiColorPolygonProgram2D* gl_LMultiColorPolygonProgram2D_new(gl_LShaderProgram* program)
{
  gl_LMultiColorPolygonProgram2D* out = malloc(sizeof(gl_LMultiColorPolygonProgram2D));

  // init defaults
  init_defaults(out);

  // init from parmeters
  out->program = program;

  return out;
}

void gl_LMultiColorPolygonProgram2D_free(gl_LMultiColorPolygonProgram2D* program)
{
  // free underlying program
  gl_LShaderProgram_free(program->program);

  // free source
  free(program);
  program = NULL;
}

bool gl_LMultiColorPolygonProgram2D_load_program(gl_LMultiColorPolygonProgram2D* program)
{
  // get underlying program
  gl_LShaderProgram* uprog = program->program;

  // generate program
  uprog->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = gl_LShaderProgram_load_shader_from_file("res/shaders/LMultiColorPolygonProgram2D.vert", GL_VERTEX_SHADER);
  // check for errors
  if (vertex_shader == 0)
  {
    glDeleteProgram(uprog->program_id);
    uprog->program_id = 0;
    return false;
  }

  // attach vertex shader to program
  glAttachShader(uprog->program_id, vertex_shader);

  // create fragment shader
  GLuint fragment_shader = gl_LShaderProgram_load_shader_from_file("res/shaders/LMultiColorPolygonProgram2D.frag", GL_FRAGMENT_SHADER);
  // check for errors
  if (fragment_shader == 0)
  {
    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete program
    glDeleteShader(uprog->program_id);
    uprog->program_id = 0;
    return false;
  }

  // attach fragment shader to program
  glAttachShader(uprog->program_id, fragment_shader);

  // link program
  glLinkProgram(uprog->program_id);
  // check for errors
  GLint link_status = GL_FALSE;
  glGetProgramiv(uprog->program_id, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    // print log from this program
    SDL_Log("Unable to link program %d.", uprog->program_id);
    gl_LShaderProgram_print_program_log(uprog->program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader);
    fragment_shader = 0;

    // delete program
    glDeleteProgram(uprog->program_id);
    uprog->program_id = 0;

    return false;
  }

  // clean up excess shader references
  glDeleteShader(vertex_shader);
  vertex_shader = 0;
  glDeleteShader(fragment_shader);
  fragment_shader = 0;

  // get variable locations
  program->vertex_pos2d_location = glGetAttribLocation(uprog->program_id, "vertex_pos2d");
  if (program->vertex_pos2d_location == -1)
  {
    SDL_Log("%s is not valid glsl program variable", "vertex_pos2d");
  }
  program->multi_color_location = glGetAttribLocation(uprog->program_id, "multi_color");
  if (program->multi_color_location == -1)
  {
    SDL_Log("%s is not valid glsl program variable", "multi_color");
  }
  program->projection_matrix_location = glGetUniformLocation(uprog->program_id, "projection_matrix");
  if (program->projection_matrix_location == -1)
  {
    SDL_Log("%s is not valid glsl program variable", "projection_matrix");
  }
  program->modelview_matrix_location = glGetUniformLocation(uprog->program_id, "modelview_matrix");
  if (program->modelview_matrix_location == -1)
  {
    SDL_Log("%s is not valid glsl program variable", "modelview_matrix");
  }

  return true;
}

void gl_LMultiColorPolygonProgram2D_update_projection_matrix(gl_LMultiColorPolygonProgram2D* program)
{
  glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]); 
}

void gl_LMultiColorPolygonProgram2D_update_modelview_matrix(gl_LMultiColorPolygonProgram2D* program)
{
  glUniformMatrix4fv(program->modelview_matrix_location, 1, GL_FALSE, program->modelview_matrix[0]);
}

void gl_LMultiColorPolygonProgram2D_set_vertex_pointer(gl_LMultiColorPolygonProgram2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void gl_LMultiColorPolygonProgram2D_set_color_pointer(gl_LMultiColorPolygonProgram2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->multi_color_location, 4, GL_FLOAT, GL_FALSE, stride, data);
}

void gl_LMultiColorPolygonProgram2D_enable_attrib_pointers(gl_LMultiColorPolygonProgram2D* program)
{
  glEnableVertexAttribArray(program->vertex_pos2d_location);
  glEnableVertexAttribArray(program->multi_color_location);
}

void gl_LMultiColorPolygonProgram2D_disable_attrib_pointers(gl_LMultiColorPolygonProgram2D* program)
{
  glDisableVertexAttribArray(program->vertex_pos2d_location);
  glDisableVertexAttribArray(program->multi_color_location);
}
