#ifndef gl_LPlainPolygonProgram2D_h_
#define gl_LPlainPolygonProgram2D_h_

#include "gl/glLOpenGL.h"
#include "gl/gl_LShaderProgram.h"
#include <stdlib.h>

typedef struct
{
  // underlying shader program pointer
  gl_LShaderProgram* program;

  // color uniform location
  // (internal use, read-only)
  GLint polygon_color_location;

  // projection matrix
  mat4 projection_matrix;

  // (internal use)
  // projection matrix location in shader
  GLint projection_matrix_location;

  // modelview matrix
  mat4 modelview_matrix;

  // (internal use)
  // modelview matrix location in shader
  GLint modelview_matrix_location;

} gl_LPlainPolygonProgram2D;

///
/// Create a new gl_LPlainPolygonProgram2D
/// Underlying gl_LShaderProgram will be automatically managed by clearing memory when done.
///
/// \param program Underlying pointer to gl_LShaderProgram
/// \return Newly created gl_LPlainPolygonProgram2D
///
extern gl_LPlainPolygonProgram2D* gl_LPlainPolygonProgram2D_new(gl_LShaderProgram* program);

///
/// Free gl_LPlainPolygonProgram2D
///
/// \param program Pointer to gl_LPlainPolygonProgram2D
///
extern void gl_LPlainPolygonProgram2D_free(gl_LPlainPolygonProgram2D* program);

///
/// Load program
///
/// \param program Program id
///
extern bool gl_LPlainPolygonProgram2D_load_program(gl_LPlainPolygonProgram2D* program);

///
/// Set polygon color
///
/// \param program Pointer to gl_LPlainPolygonProgram2D
/// \param r Red color component between 0.0-1.0
/// \param g Green color component between 0.0-1.0
/// \param b Blue color component between 0.0-1.0
/// \param a Alpha color component between 0.0-1.0
///
extern void gl_LPlainPolygonProgram2D_set_color(gl_LPlainPolygonProgram2D* program, GLfloat r, GLfloat g, GLfloat b, GLfloat a);

///
/// Update projection matrix to shader with projection matrix that has been set.
///
/// \param program Pointer to gl_LPlainPolygonProgram2D
///
extern void gl_LPlainPolygonProgram2D_update_projection_matrix(gl_LPlainPolygonProgram2D* program);

///
/// Update modelview matrix to shader with modelview matrix that has been set.
///
/// \param program Pointer to gl_LPlainPolygonProgram2D
///
extern void gl_LPlainPolygonProgram2D_update_modelview_matrix(gl_LPlainPolygonProgram2D* program);

#endif
