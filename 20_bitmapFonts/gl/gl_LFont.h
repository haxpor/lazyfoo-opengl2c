#ifndef gl_LFont_h_
#define gl_LFont_h_

#include "glLOpenGL.h"
#include "gl_LTexture_spritesheet.h"

typedef struct
{
  /// underlying spritesheet
  gl_LSpritesheet* spritesheet;

  // spacing variables
  /// how much spacing when found ' ' space
  GLfloat space;
  // how much spacing between highest text pixel to lowest text pixel
  GLfloat line_height;
  // how much spacing when found '\n' (newline)
  GLfloat newline;
} gl_LFont;

///
/// Create a new bitmap font.
/// gl_LSpritesheet will be managed and automatically freed memory when done.
/// gl_LSpritesheet as input needs not to be generated data buffer just yet.
///
/// \param spritesheet Pointer to gl_LSpritesheet
/// \return Newly created gl_LFont allocated on heap.
///
extern gl_LFont* gl_LFont_new(gl_LSpritesheet* spritesheet);

///
/// Free font.
/// After this call, gl_LFont will be freed (destroyed).
///
/// \param font Pointer to gl_LFont
///
extern void gl_LFont_free(gl_LFont* font);

///
/// Load bitmap
///
/// \param font Pointer to gl_LFont
/// \param path Path to bitmap file to load
/// \return True if successfully load, otherwise return false
///
extern bool gl_LFont_load_bitmap(gl_LFont* font, const char* path);

///
/// Free gl_LFont's font.
/// This doesn't free or destroy gl_LFont itself.
///
/// \param font Pointer to gl_LFont
///
extern void gl_LFont_free_font(gl_LFont* font);

///
/// Render text
///
/// \param font Pointer to gl_LFont
/// \param text Text to render
/// \param x Position x to render. Origin is at top-left.
/// \param y Position y to render. Origin is at top-left.
///
extern void gl_LFont_render_text(gl_LFont* font, const char* text, GLfloat x, GLfloat y);

#endif
