#ifndef gl_LMultiColorPolygonProgram2D_h_
#define gl_LMultiColorPolygonProgram2D_h_

#include "gl/glLOpenGL.h"
#include "gl/gl_LShaderProgram.h"

typedef struct
{
  // underlying shader program
  gl_LShaderProgram* program;

  // vertex position 2d location
  // (internal use)
  GLint vertex_pos2d_location;

  // multi color location
  // (internal use)
  GLint multi_color_location;

  // projection matrix
  mat4 projection_matrix;

  // projection matrix location
  // (internal use)
  GLint projection_matrix_location;

  // modelview matrix
  mat4 modelview_matrix;

  // modelview matrix location
  // (internal use)
  GLint modelview_matrix_location;

} gl_LMultiColorPolygonProgram2D;

///
/// Create a new multi-color polygon program.
/// Input gl_LShaderProgram will be automatically managed its memory freeing. User has no responsible to manually free it later.
///
/// \param program Pointer to gl_LShaderProgram
/// \return Newly created gl_LMultiColorPolygonProgram2D on heap
///
extern gl_LMultiColorPolygonProgram2D* gl_LMultiColorPolygonProgram2D_new(gl_LShaderProgram* program);

///
/// Free multi-color program
///
/// \param program Pointer to gl_LMultiColorPolygonProgram2D
///
extern void gl_LMultiColorPolygonProgram2D_free(gl_LMultiColorPolygonProgram2D* program);

///
/// Load program
///
/// \param Pointer to gl_LMultiColorPolygonProgram2D
/// \return True if load successfully, otherwise return false.
///
extern bool gl_LMultiColorPolygonProgram2D_load_program(gl_LMultiColorPolygonProgram2D* program);

///
/// Update projection matrix by sending to shader.
///
/// \param program Pointer to gl_LMultiColorPolygonProgram2D
///
extern void gl_LMultiColorPolygonProgram2D_update_projection_matrix(gl_LMultiColorPolygonProgram2D* program);

///
/// Update modelview matrix by sending to shader.
///
/// \param program Pointer to gl_LMultiColorPolygonProgram2D
///
extern void gl_LMultiColorPolygonProgram2D_update_modelview_matrix(gl_LMultiColorPolygonProgram2D* program);

///
/// Set vertex pointer.
///
/// \param program Pointer to LMultiColorPolygonProgram2D
/// \param stride Size in bytes of each vertex element
/// \param data Opaque pointer to data buffer
///
extern void gl_LMultiColorPolygonProgram2D_set_vertex_pointer(gl_LMultiColorPolygonProgram2D* program, GLsizei stride, const GLvoid* data);

///
/// set color pointer
///
/// \param program pointer to gl_LMultiColorPolygonProgram2D
/// \param size in bytes of each vertex element
/// \param opaque pointer to data buffer
///
extern void gl_LMultiColorPolygonProgram2D_set_color_pointer(gl_LMultiColorPolygonProgram2D* program, GLsizei stride, const GLvoid* data);

///
/// enable all relevant generic vertex array pointers to this program
///
/// \param pointer to gl_LMultiColorPolygonProgram2D
///
extern void gl_LMultiColorPolygonProgram2D_enable_attrib_pointers(gl_LMultiColorPolygonProgram2D* program);

///
/// disable all relevant generic vertex array  pointers to this program
///
/// \param program pointer to gl_LMultiColorPolygonProgram2D
///
extern void gl_LMultiColorPolygonProgram2D_disable_attrib_pointers(gl_LMultiColorPolygonProgram2D* program);

#endif
