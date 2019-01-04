#ifndef gl_LPlainPolygonProgram2D_h_
#define gl_LPlainPolygonProgram2D_h_

#include "gl/glLOpenGL.h"
#include "gl/gl_LShaderProgram.h"
#include <stdlib.h>

// for now there's no additional attribute on top of
// base one which is gl_LShaderProgram
typedef gl_LShaderProgram gl_LPlainPolygonProgram2D;

///
/// Create a new gl_LPlainPolygonProgram2D
///
/// \return Newly created gl_LPlainPolygonProgram2D
///
extern gl_LPlainPolygonProgram2D* gl_LPlainPolygonProgram2D_new();

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

#endif
