#include "gl_util.h"
#include "glLOpenGL.h"
#include <stdio.h>
#include "SDL_log.h"

void gl_util_adapt_to_normal(int screen_width, int screen_height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
  glOrtho(0.0, screen_width, screen_height, 0.0, -1.0, 1.0);
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

	SDL_Log("viewport_x: %d, viewport_y: %d", viewport_x, viewport_y);
	SDL_Log("view_width: %d, view_height: %d", view_width, view_height);

	SDL_Log("view_width/logical_width: %f", view_width * 1.0f / logical_width);
	SDL_Log("view_height/logical_height: %f", view_height * 1.0f / logical_height);

	// set orthographic projection matrix
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
	glOrtho(0.0, view_width, view_height, 0.0, -1.0, 1.0);

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
