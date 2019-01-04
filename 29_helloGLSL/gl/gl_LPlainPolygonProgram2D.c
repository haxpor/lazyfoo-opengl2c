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

  // create vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

  // get vertex source
  // set array of pointer to string, consule manpage of glShaderSource()
  const GLchar* vertex_shader_source[] = 
  {
    "void main()"
    "{"
    "   gl_Position = gl_Vertex;"
    "}" 
  };

  // set vertex source
  glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);

  // compile vertex source
  glCompileShader(vertex_shader);

  // check vertex shader for errors
  GLint shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_compiled);
  if (shader_compiled != GL_TRUE)
  {
    SDL_Log("Unable to compile vertex shader: %d", vertex_shader);
    gl_LShaderProgram_print_shader_log(vertex_shader);

    // delete shader object
    glDeleteShader(vertex_shader);
    vertex_shader = 0;
    return false;
  }

  // attach vertex shader to program
  glAttachShader(program->program_id, vertex_shader);
  // delete shader object
  glDeleteShader(vertex_shader);
  vertex_shader = 0;

  // create fragment shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  // get fragement source
  const GLchar* fragment_shader_source[] =
  {
    "void main()"
    "{"
    " gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
    "}"
  };

  // set fragment source
  glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);

  // compile fragment source
  glCompileShader(fragment_shader);

  // check fragment shader for errors
  GLint fragment_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compiled);
  if (fragment_compiled != GL_TRUE)
  {
    SDL_Log("Unable to compile fragment shader: %d", fragment_shader);
    gl_LShaderProgram_print_shader_log(fragment_shader);

    // delete shader object
    glDeleteShader(fragment_shader);
    fragment_shader = 0;
    return false;
  }

  // attach fragment shader to program
  glAttachShader(program->program_id, fragment_shader);
  // delete shader
  glDeleteShader(fragment_shader);
  fragment_shader = 0;

  // link program
  glLinkProgram(program->program_id);

  // check for errors
  GLint link_status = GL_FALSE;
  glGetProgramiv(program->program_id, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    SDL_Log("Unable to link shader program: %d", program->program_id);
    gl_LShaderProgram_print_program_log(program->program_id);

    return false;
  }

  return true;
}
