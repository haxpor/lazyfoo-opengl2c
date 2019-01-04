#ifndef gl_LShaderProgram_h_
#define gl_LShaderProgram_h_

#include "gl/glLOpenGL.h"
#include <stdbool.h>

typedef struct
{
  // program id
  GLuint program_id;
} gl_LShaderProgram;

///
/// Create a new shader program.
///
/// \return Newly created gl_LShaderProgram returned as pointer to gl_LShaderProgram
///
extern gl_LShaderProgram* gl_LShaderProgram_new();

///
/// Free shader program.
///
/// \param shader_program Pointer to gl_LShaderProgram
///
extern void gl_LShaderProgram_free(gl_LShaderProgram* shader_program);

///
/// Load shader from file according to type.
///
/// \param path Path to shader file
/// \param shader_type Type of shader
/// \return Return id of a compiled shader, or 0 if failed.
///
extern GLuint gl_LShaderProgram_load_shader_from_file(const char* path, GLenum shader_type);

///
/// Free shader program
///
/// \param shader_program Pointer to gl_LShaderProgram
///
extern void gl_LShaderProgram_free_program(gl_LShaderProgram* shader_program);

///
/// Bind this shader program, thus set this shader program as the current shader program
///
/// \param shader_program Pointer to gl_LShaderProgram
/// \return True if bind successfully, otherwise return false
///
extern bool gl_LShaderProgram_bind(gl_LShaderProgram* shader_program);

///
/// Unbind this shader program, thus unset it as the current shader program.
///
/// \param shader_program Pointer to gl_LShaderProgram
///
extern void gl_LShaderProgram_unbind(gl_LShaderProgram* shader_program);

///
/// Print out log for input program id (or say program name).
///
/// \param program_id Program id to print log
///
extern void gl_LShaderProgram_print_program_log(GLuint program_id);

///
/// Print out log for input shader id (or say shader name).
///
/// \param shader_id Shader id to print log
///
extern void gl_LShaderProgram_print_shader_log(GLuint shader_id);

#endif
