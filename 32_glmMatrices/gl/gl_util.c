#include "gl_util.h"
#include <stdio.h>
#include "SDL_log.h"

void gl_util_adapt_to_normal(int screen_width, int screen_height)
{
	// set viewport
	glViewport(0.0, 0.0, screen_width, screen_height);
}

void gl_util_adapt_to_letterbox(int screen_width, int screen_height, int logical_width, int logical_height, int* result_view_width, int* result_view_height, int* offset_x, int* offset_y)
{
  // coordinate is different from OpenGL, Y+ is up, and Y- is down; opposite of opengl
	float aspect = logical_width * 1.0f / logical_height;

	// prioritize base on height of screen's height first
	int view_height = screen_height;
	int view_width = screen_height * aspect;

	// if view's width needs more space to show all game content and cannot fit
	// into current screen's width then we need to do another approach to base
	// on width instead
	// note: less likely to happen as most game would letterbox on the horizontal side not on vertical side
	if (view_width > screen_width)
	{
		view_width = screen_width;
		view_height = screen_width / aspect;
	}

	int viewport_x = (screen_width - view_width) / 2;
	int viewport_y = (screen_height - view_height) / 2;

	// set viewport
  glViewport(viewport_x, viewport_y, view_width, view_height);

	// returning values back from functions via variables
	if (result_view_width != NULL)
	{
		*result_view_width = view_width;
	}
	if (result_view_height != NULL)
	{
		*result_view_height = view_height;
	}
	if (offset_x != NULL)
	{
		*offset_x = viewport_x;
	}
	if (offset_y != NULL)
	{
		*offset_y = viewport_y;
	}
}

// implementation note: decided to return string literal instead of using global string variable
// as to reduce management effort in code to clear error message when execute next related command
const char* gl_util_error_string(GLenum error)
{
  if (error == GL_NO_ERROR)
    return "No error";
  else if (error == GL_INVALID_ENUM)
    return "An unacceptable value is specified for an enumerated argument";
  else if (error == GL_INVALID_VALUE)
    return "A numeric argument is out of range";
  else if (error == GL_INVALID_OPERATION)
    return "The specified operation is not allowed in the current state";
  else if (error == GL_STACK_OVERFLOW)
    return "This command would cause a stack overflow";
  else if (error == GL_STACK_UNDERFLOW)
    return "This command would cause a stack underflow";
  else if (error == GL_OUT_OF_MEMORY)
    return "There is not enough memory left to execute the command";
  else if (error == GL_TABLE_TOO_LARGE)
    return " The specified table exceeds the implementation's maximum supported table size";
  else
    return "Unknown error"; // should not happen anyway if you get error from glGetError()
}

GLuint gl_util_map_color_RGBA_to_ABGR(GLuint color)
{
  // map color key to ABGR format
  GLubyte* color_key_bytes = (GLubyte*)&color;
  // we get sequence of bytes which ranging from least to most significant so we can use it right away
  return (color_key_bytes[0] << 24) | (color_key_bytes[1] << 16) | (color_key_bytes[2] << 8) | (color_key_bytes[3]); 
}
