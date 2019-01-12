#ifndef gl_lfont_polygon_program2d_h_
#define gl_lfont_polygon_program2d_h_

#include "gl/glLOpenGL.h"
#include "gl/gl_LShaderProgram.h"

typedef struct gl_lfont_polygon_program2d_
{
  // underlying shader program
  gl_LShaderProgram *program;

  /// attribute locations
  GLint vertex_pos2d_location;
  GLint texture_coord_location;

  /// uniform location
  /// (internal use)
  GLint projection_matrix_location;
  GLint modelview_matrix_location;
  GLint texture_sampler_location;
  GLint text_color_location;

  /// matrices
  mat4 projection_matrix;
  mat4 modelview_matrix;

} gl_lfont_polygon_program2d;

///
/// create a new font shader program on heap.
/// it will also create underlying gl_LShaderProgram and manage it automatically for its memory deallocation.
///
/// \return Newly created gl_lfont_polygon_program2d
///
extern gl_lfont_polygon_program2d* gl_lfont_polygon_program2d_new();

///
/// free font shader program
///
/// \param program pointer to gl_lfont_polygon_program2d
///
extern void gl_lfont_polygon_program2d_free(gl_lfont_polygon_program2d* program);

///
/// load program
///
/// \param program pointer to program
/// \return true if load successfully, otherwise false
///
extern bool gl_lfont_polygon_program2d_load_program(gl_lfont_polygon_program2d* program);

///
/// update projection matrix then to update to gpu.
///
/// \param program pointer to program
///
extern void gl_lfont_polygon_program2d_update_projection_matrix(gl_lfont_polygon_program2d* program);

///
/// update modelview matrix then to update to gpu.
///
/// \param program pointer to program
///
extern void gl_lfont_polygon_program2d_update_modelview_matrix(gl_lfont_polygon_program2d* program);

///
/// set vertex pointer then to update to gpu
///
/// \param program pointer to program
/// \param stride byte spaces til next element
/// \param data pointer to data buffer offset
///
extern void gl_lfont_polygon_program2d_set_vertex_pointer(gl_lfont_polygon_program2d* program, GLsizei stride, const GLvoid* data);

///
/// set texture coordinate pointer then to update to gpu
///
/// \param program pointer to program
/// \param stride byte spaces til next element
/// \param data pointer to data buffer offset
///
extern void gl_lfont_polygon_program2d_set_texcoord_pointer(gl_lfont_polygon_program2d* program, GLsizei stride, const GLvoid* data);

///
/// set texture sampler name then to update to gpu
///
/// \param program pointer to program
/// \param sampler sampler name to bind texture
///
extern void gl_lfont_polygon_program2d_set_texture_sampler(gl_lfont_polygon_program2d* program, GLuint sampler);

///
/// set text color, then to update to gpu.
///
/// \param program pointer to program
/// \param color text color
///
extern void gl_lfont_polygon_program2d_set_text_color(gl_lfont_polygon_program2d* program, LColorRGBA color);

///
/// enable all attribute pointers
///
/// \parm program pointer to program
///
extern void gl_lfont_polygon_program2d_enable_attrib_pointers(gl_lfont_polygon_program2d* program);

///
/// disable all attribute pointers
///
/// \param program pointer to program
///
extern void gl_lfont_polygon_program2d_disable_attrib_pointers(gl_lfont_polygon_program2d* program);

#endif
