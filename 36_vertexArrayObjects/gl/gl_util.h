#ifndef gl_util_h_
#define gl_util_h_

#include "glLOpenGL.h"

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

///
/// Map color from RGBA to ABGR.
///
/// \param color Input color in RGBA
/// \return Mapped color in ABGR
///
extern GLuint gl_util_map_color_RGBA_to_ABGR(GLuint color);

///
/// check and print any error so far for opengl
/// if opengl has any error, it will print error message on screen then return such error code
///
/// \param prefix prefix text to print. can be NULL.
/// \return error if any, or GL_NO_ERROR if no
///
extern GLenum gl_util_anyerror(const char* prefix);

///
/// Update projection matrix at the location then issue update to GPU.
///
/// \param location location of uniform variable in shader code
/// \param matrix projection matrix to update
///
extern void gl_util_update_projection_matrix(GLint location, mat4 matrix);

///
/// Update modelview matrix at the location then issue update to GPU.
///
/// \param location location of uniform variable in shader code
/// \param matrix modelview matrix to update
///
extern void gl_util_update_modelview_matrix(GLint location, mat4 matrix);

///
/// Enable vertex attribute pointers from input variable of locations.
/// Specify -1 to end the variadic input.
///
/// \param location location to enable in variadic.
///
extern void gl_util_enable_vertex_attrib_pointers(GLint location, ...);

///
/// Disable vertex attribute pointers from input variable of locations.
/// Specify -1 to end the variadic input.
///
/// \param location location to disable in variadic.
///
extern void gl_util_disable_vertex_attrib_pointers(GLint location, ...);

#endif
