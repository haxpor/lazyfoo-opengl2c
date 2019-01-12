#ifndef gl_ldouble_multicolor_polygon_program2d_h_
#define gl_ldouble_multicolor_polygon_program2d_h_

#include "gl/glLOpenGL.h"
#include "gl/gl_LShaderProgram.h"

typedef struct gl_ldouble_multicolor_polygon_program2d_
{
  /// underlying shader program
  gl_LShaderProgram* program;

  /// attribute location
  GLint vertex_pos2d_location;
  GLint multicolor1_location;
  GLint multicolor2_location;

  /// uniform location
  /// (internal use)
  GLint projection_matrix_location;
  GLint modelview_matrix_location;

  // matrices
  mat4 projection_matrix;
  mat4 modelview_matrix;

} gl_ldouble_multicolor_polygon_program2d;

///
/// create a new double multi-color shader program.
/// it will also create and manage underlying program (gl_LShaderProgram).
/// user has no responsibility to free its underlying attribute again.
///
/// \return Newly created gl_ldouble_multicolor_polygon_program2d on heap.
///
extern gl_ldouble_multicolor_polygon_program2d* gl_ldouble_multicolor_polygon_program2d_new();

///
/// Free internals
///
/// \param program pointer to gl_ldouble_multicolor_polygon_program2d
///
extern void gl_ldouble_multicolor_polygon_program2d_free_internals(gl_ldouble_multicolor_polygon_program2d* program);

///
/// free double multi-color shader program.
///
/// \param program pointer to gl_ldouble_multicolor_polygon
extern void gl_ldouble_multicolor_polygon_program2d_free(gl_ldouble_multicolor_polygon_program2d* program);

///
/// load program
///
/// \param program pointer to gl_ldouble_multicolor_polygon_program2d
///
extern bool gl_ldouble_multicolor_polygon_program2d_load_program(gl_ldouble_multicolor_polygon_program2d* program);

///
/// Enable all vertex attribute pointers
///
/// \param program pointer to program
#define gl_ldouble_multicolor_polygon_program2d_enable_all_vertex_attrib_pointers(program) gl_util_enable_vertex_attrib_pointers(program->vertex_pos2d_location, program->multicolor1_location, program->multicolor2_location, -1)

///
/// Disable all vertex attribute pointers
///
/// \param program pointer to program
///
#define gl_ldouble_multicolor_polygon_program2d_disable_all_vertex_attrib_pointers(program) gl_util_disable_vertex_attrib_pointers(program->vertex_pos2d_location, program->multicolor1_location, program->multicolor2_location, -1)

/// set vertex pointer (packed version)
/// it will set stride as 0 as packed format.
/// if caller intend to use a single VBO combining several vertex data type together then this function is not the one you're looking for.
/// data - offset pointer to data
#define gl_ldouble_multicolor_polygon_program2d_set_attrib_vertex_pos2d_pointer_packed(program, data) glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)data)

/// set attrib multicolor either 1 or 2 (packed version)
/// program - shader program
/// color - 1 for multicolor-1, 2 for multicolor-2
/// data - offset pointer to data
#define gl_ldouble_multicolor_polygon_program2d_set_attrib_multicolor_pointer_packed(program, color, data) glVertexAttribPointer(color == 1 ? program->multicolor1_location : program->multicolor2_location, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)data)

#endif
