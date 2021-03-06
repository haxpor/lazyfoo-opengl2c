#include "gl/gl_LPlainPolygonProgram2D.h"
#include "gl/gl_LShaderProgram_internals.h"
#include <stdlib.h>
#include "SDL_log.h"

static void init_defaults(gl_LPlainPolygonProgram2D* program);

void init_defaults(gl_LPlainPolygonProgram2D* program)
{
  program->polygon_color_location = 0;
}

gl_LPlainPolygonProgram2D* gl_LPlainPolygonProgram2D_new(gl_LShaderProgram* program)
{
  gl_LPlainPolygonProgram2D* out = malloc(sizeof(gl_LPlainPolygonProgram2D));
  out->program = program;

  // init defaults
  init_defaults(out);
  gl_LShaderProgram_init_defaults(out->program);

  return out;
}

void gl_LPlainPolygonProgram2D_free(gl_LPlainPolygonProgram2D* program)
{
  gl_LShaderProgram_free(program->program);
}

bool gl_LPlainPolygonProgram2D_load_program(gl_LPlainPolygonProgram2D* program)
{
  // generate program
  program->program->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = gl_LShaderProgram_load_shader_from_file("res/shaders/LPlainPolygonProgram2D.vert", GL_VERTEX_SHADER);

  // check for errors
  if (vertex_shader == 0)
  {
    // delete program
    glDeleteProgram(program->program->program_id);
    program->program->program_id = 0;
    return false;
  }
  // attach vertex shader to program
  glAttachShader(program->program->program_id, vertex_shader);

  // create fragment shader
  GLuint fragment_shader = gl_LShaderProgram_load_shader_from_file("res/shaders/LPlainPolygonProgram2D.frag", GL_FRAGMENT_SHADER);

  // check for errors
  if (fragment_shader == 0)
  {
    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete program
    glDeleteProgram(program->program->program_id);
    program->program->program_id = 0;
    return false;
  }

  // attach fragment shader to program
  glAttachShader(program->program->program_id, fragment_shader);

  // link program
  glLinkProgram(program->program->program_id);

  // check for errors
  GLint link_status = GL_FALSE;
  glGetProgramiv(program->program->program_id, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    SDL_Log("Error linking program %d", program->program->program_id);
    gl_LShaderProgram_print_program_log(program->program->program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader);
    fragment_shader = 0;

    // delete program
    glDeleteProgram(program->program->program_id);
    program->program->program_id = 0;
    return false;
  }

  // clean up
  // delete vertex shader
  glDeleteShader(vertex_shader);
  vertex_shader = 0;

  // delete fragment shader
  glDeleteShader(fragment_shader);
  fragment_shader = 0;

  // get variable location
  // note: we can only query for location after link successfully
  program->polygon_color_location = glGetUniformLocation(program->program->program_id, "polygon_color");
  if (program->polygon_color_location == -1)
  {
    SDL_Log("%s is not a valid glsl program variable", "polygon_color");
  }

  return true;
}

void gl_LPlainPolygonProgram2D_set_color(gl_LPlainPolygonProgram2D* program, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  // update color in shader
  glUniform4f(program->polygon_color_location, r, g, b, a);
}
