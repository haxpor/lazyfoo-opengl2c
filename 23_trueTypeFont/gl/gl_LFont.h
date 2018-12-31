#ifndef gl_LFont_h_
#define gl_LFont_h_

#include "glLOpenGL.h"
#include "gl_LTexture_spritesheet.h"
#include "ft2build.h"
#include FT_FREETYPE_H

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
/// \return Newly created gl_LFont allocated on heap. It can be NULL if underlying system initialization failed to do so.
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
/// Expects 8-bit grayscale image with no alpha.
///
/// \param font Pointer to gl_LFont
/// \param path Path to bitmap file to load
/// \return True if successfully load, otherwise return false
///
extern bool gl_LFont_load_bitmap(gl_LFont* font, const char* path);

///
/// Load FreeType font
///
/// \param font Pointer to gl_LFont
/// \param path Path to load TTF file
/// \param pixel_size Pixel size of font to generate bitmap font from TTF file
/// \return True if load successfully, otherwise return false.
///
extern bool gl_LFont_load_freetype(gl_LFont* font, const char* path, GLuint pixel_size);

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
