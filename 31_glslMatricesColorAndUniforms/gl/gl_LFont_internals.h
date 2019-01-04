#ifndef gl_LFont_internals_h_
#define gl_LFont_internals_h_

#include "gl/glLOpenGL.h"
#include "gl/gl_LFont.h"

/// This header is meant to be used internally by library itself.
/// If you include this, you should know what you're doing.

///
/// Return the width of input string according to the current loaded font.
/// It will return the width until it reaches '\n' or '\0'.
///
/// \param font Pointer to gl_LFont
/// \param string Input string
/// \return String width
///
extern GLfloat gl_LFont_string_width(gl_LFont* font, const char* string);

///
/// Return string's height required to render it according to current loaded font.
///
/// \param font Pointer to gl_LFont
/// \param string Input string
/// \return String's height
///
extern GLfloat gl_LFont_string_height(gl_LFont* font, const char* string);

#endif
