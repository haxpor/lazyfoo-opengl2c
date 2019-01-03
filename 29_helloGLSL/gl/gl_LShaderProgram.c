#include "gl_LShaderProgram.h"
#include "gl_LShaderProgram_internals.h"
#include "gl/gl_util.h"
#include <stdlib.h>
#include "SDL_log.h"

void gl_LShaderProgram_init_defaults(gl_LShaderProgram* shader_program)
{
  shader_program->program_id = 0;
}

gl_LShaderProgram* gl_LShaderProgram_new()
{
  gl_LShaderProgram* out = malloc(sizeof(gl_LShaderProgram));
  gl_LShaderProgram_init_defaults(out);

  return out;
}

void gl_LShaderProgram_free(gl_LShaderProgram* shader_program)
{
  // free program
  gl_LShaderProgram_free_program(shader_program);

  // free the source
  free(shader_program);
  shader_program = NULL;
}

void gl_LShaderProgram_free_program(gl_LShaderProgram* shader_program)
{
  // delete program
  glDeleteProgram(shader_program->program_id);
  shader_program->program_id = 0;
}

bool gl_LShaderProgram_bind(gl_LShaderProgram* shader_program)
{
  // use shader
  glUseProgram(shader_program->program_id);

  // check for error
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error use program: %s", gl_util_error_string(error));
    return false;
  }

  return true;
}

void gl_LShaderProgram_unbind(gl_LShaderProgram* shader_program)
{
  // use default program
  glUseProgram(0);
}

void gl_LShaderProgram_print_program_log(GLuint program_id)
{
  // make sure name is shader
  if (glIsProgram(program_id))
  {
    // program log length
    int info_log_length = 0;
    int max_length = info_log_length;

    // get info string length
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &max_length);

    // allocate string
    char info_log[max_length];

    // get info log
    glGetProgramInfoLog(program_id, max_length, &info_log_length, info_log);
    if (info_log_length > 0)
    {
      // print log
      SDL_Log("%s", info_log);
    }
  }
  else
  {
    SDL_Log("Name %d is not a program", program_id);
  }
}

void gl_LShaderProgram_print_shader_log(GLuint shader_id)
{
  // make sure name is shader
  if (glIsShader(shader_id))
  {
    // shader log length
    int info_log_length = 0;
    int max_length = info_log_length;

    // get info string length
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);

    // allocate string
    char info_log[max_length];

    // get info log
    glGetShaderInfoLog(shader_id, max_length, &info_log_length, info_log);
    if (info_log_length > 0)
    {
      // print log
      SDL_Log("%s", info_log);
    }
  }
  else
  {
    SDL_Log("Name %d is not a shader", shader_id);
  }
}
