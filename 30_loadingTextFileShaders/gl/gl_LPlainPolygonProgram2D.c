#include "gl/gl_LPlainPolygonProgram2D.h"
#include "gl/gl_LShaderProgram_internals.h"
#include <stdlib.h>
#include "SDL_log.h"

gl_LPlainPolygonProgram2D* gl_LPlainPolygonProgram2D_new()
{
  gl_LPlainPolygonProgram2D* out = malloc(sizeof(gl_LPlainPolygonProgram2D));

  // init defaults
  // just relay the function call for now
  gl_LShaderProgram_init_defaults((gl_LShaderProgram*)out);

  return out;
}

void gl_LPlainPolygonProgram2D_free(gl_LPlainPolygonProgram2D* program)
{
  // just relay the function call for now
  gl_LShaderProgram_free((gl_LShaderProgram*)program);
}

bool gl_LPlainPolygonProgram2D_load_program(gl_LPlainPolygonProgram2D* program)
{
  // generate program
  program->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = gl_LShaderProgram_load_shader_from_file("res/shaders/LPlainPolygonProgram2D.vert", GL_VERTEX_SHADER);

  // check for errors
  if (vertex_shader == 0)
  {
    // delete program
    glDeleteProgram(program->program_id);
    program->program_id = 0;
    return false;
  }
  // attach vertex shader to program
  glAttachShader(program->program_id, vertex_shader);

  // create fragment shader
  GLuint fragment_shader = gl_LShaderProgram_load_shader_from_file("res/shaders/LPlainPolygonProgram2D.frag", GL_FRAGMENT_SHADER);

  // check for errors
  if (fragment_shader == 0)
  {
    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete program
    glDeleteProgram(program->program_id);
    program->program_id = 0;
    return false;
  }

  // attach fragment shader to program
  glAttachShader(program->program_id, fragment_shader);

  // link program
  glLinkProgram(program->program_id);

  // check for errors
  GLint link_status = GL_FALSE;
  glGetProgramiv(program->program_id, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    SDL_Log("Error linking program %d", program->program_id);
    gl_LShaderProgram_print_program_log(program->program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader);
    fragment_shader = 0;

    // delete program
    glDeleteProgram(program->program_id);
    program->program_id = 0;
    return false;
  }

  // clean up
  // delete vertex shader
  glDeleteShader(vertex_shader);
  vertex_shader = 0;

  // delete fragment shader
  glDeleteShader(fragment_shader);
  fragment_shader = 0;

  return true;
}
