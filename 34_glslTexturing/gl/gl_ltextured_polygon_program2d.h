#ifndef gl_ltextured_polygon_program2d_h_
#define gl_ltextured_polygon_program2d_h_

#include "gl/glLOpenGL.h"
#include "gl/gl_LShaderProgram.h"

typedef struct gl_ltextured_polygon_program2d_
{
  // underlying shader program
  gl_LShaderProgram* program;

  // attribute location
  GLint vertex_pos2d_location;
  GLint texcoord_location;

  // uniform color to apply to texture's pixels
  GLint texture_color_location;
  // uniform texture
  GLint texture_sampler_location;

  // projection matrix
  mat4 projection_matrix;
  GLint projection_matrix_location;

  // modelview matrix
  mat4 modelview_matrix;
  GLint modelview_matrix_location;

} gl_ltextured_polygon_program2d;

///
/// create a new textured polygon shader.
/// it will automatically create underlying gl_LShaderProgram for us.
/// its underlying gl_LShaderProgram will be managed automatically, use has no need to manually free it again.
///
/// \return Newly created gl_ltextured_polygon_program2d on heap.
///
extern gl_ltextured_polygon_program2d* gl_ltextured_polygon_program2d_new();

///
/// Free gl_ltextured_polygon_program2d.
/// after this its underlying gl_LShaderProgram will be freed as well.
///
/// \param program pointer to gl_ltextured_polygon_program2d
///
extern void gl_ltextured_polygon_program2d_free(gl_ltextured_polygon_program2d* program);

///
/// load program
///
/// \param program pointer to gl_ltextured_polygon_program2d
/// \return true if load successfully, otherwise retrurn false.
///
extern bool gl_ltextured_polygon_program2d_load_program(gl_ltextured_polygon_program2d* program);

///
/// update projection matrix
///
/// \param program pointer to gl_ltextured_polygon_program2d
///
extern void gl_ltextured_polygon_program2d_update_projection_matrix(gl_ltextured_polygon_program2d* program);

///
/// update modelview matrix
///
/// \param program pointer to gl_ltextured_polygon_program2d
///
extern void gl_ltextured_polygon_program2d_update_modelview_matrix(gl_ltextured_polygon_program2d* program);

///
/// set vertex pointer
///
/// \param program pointer to gl_ltextured_polygon_program2d
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer
///
extern void gl_ltextured_polygon_program2d_set_vertex_pointer(gl_ltextured_polygon_program2d* program, GLsizei stride, const GLvoid* data);

///
/// set texcoordinate pointer
///
/// \param program pointer to gl_ltextured_polygon_program2d
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer
///
extern void gl_ltextured_polygon_program2d_set_texcoord_pointer(gl_ltextured_polygon_program2d* program, GLsizei stride, const GLvoid* data);

///
/// set texture color.
/// color will be apply to all of texture's color.
///
/// \param program pointer to gl_ltextured_polygon_program2d
/// \param color color
///
extern void gl_ltextured_polygon_program2d_set_texture_color(gl_ltextured_polygon_program2d* program, LColorRGBA color);

///
/// set texture sampler to shader
///
/// \param program pointer to gl_ltextured_polygon_program2d
/// \param sampler texture sampler name
///
extern void gl_ltextured_polygon_program2d_set_texture_sampler(gl_ltextured_polygon_program2d* program, GLuint sampler);

///
/// enable all attribute pointers
///
/// \param program pointer to gl_ltextured_polygon_program2d
///
extern void gl_ltextured_polygon_program2d_enable_attrib_pointers(gl_ltextured_polygon_program2d* program);

///
/// disable all attribute pointers
///
/// \param program pointer to gl_ltextured_polygon_program2d
///
extern void gl_ltextured_polygon_program2d_disable_attrib_pointers(gl_ltextured_polygon_program2d* program);

#endif
