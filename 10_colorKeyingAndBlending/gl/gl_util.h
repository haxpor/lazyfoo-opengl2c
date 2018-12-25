#ifndef gl_util_h_
#define gl_util_h_

#include "gl.h"

/// Utility functions to work with OpenGL

///
/// Adapt projection matrix to normal.
///
/// \param screen_width Screen width
/// \param screen_height Screen height
///
extern void gl_util_adapt_to_normal(int screen_width, int screen_heght);

///
/// Adapt projection matrix to letterbox.
/// It can do a letterbox either on horizontal or vertical direction.
/// It depends on user's input screen and logical dimentions, and code to interpret them.
///
/// \param screen_width Screen width
/// \param screen_height Screen height
/// \param logical_width Logical width for game logic
/// \param logical_height Logical height for game logic
/// \param view_width Result of new screen width
/// \param view_height Result of new screen height
/// \param offset_x Offset x on screen for main content to be rendered. If NULL, value won't get return.
/// \param offset_y Offset y on screen for main content ot be rendered. If NULL, value won't get return.
///
extern void gl_util_adapt_to_letterbox(int screen_width, int screen_height, int logical_width, int logical_height, int* view_width, int* view_height, int* offset_x, int* offset_y);

///
/// Get error string.
///
/// \param error GLenum
/// \return Error string. You should not modify or free this string.
///
extern const char* gl_util_error_string(GLenum error);

#endif
