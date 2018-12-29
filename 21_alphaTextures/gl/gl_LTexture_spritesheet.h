#ifndef gl_LTexture_spritesheet_h_
#define gl_LTexture_spritesheet_h_

#include "glLOpenGL.h"
#include "gl_types.h"
#include "gl_LTexture.h"
#include "foundation/vector.h"

typedef struct
{
  gl_LTexture* ltexture;
  vector* clips;
  
  /// (internal use)
  GLuint vertex_data_buffer;
  /// (internal use)
  GLuint* index_buffers;
} gl_LSpritesheet;

///
/// Create a new gl_LSpritesheet.
/// Input ltexture will be taken care of when it needs to be freed later in the process.
///
/// \param ltexture Pointer to already created gl_LTexture. This gl_LTexture will be taken care off later on when it needs to be freed.
/// \return Newly created gl_LSpritesheet
///
extern gl_LSpritesheet* gl_LSpritesheet_new(gl_LTexture* ltexture);

///
/// Free spritesheeet.
/// It will also free its gl_LTexture.
///
/// \param spritesheet Pointer to spritesheet
///
extern void gl_LSpritesheet_free(gl_LSpritesheet* spritesheet);

///
/// Add a new clipping for sprite inside spritesheet.
///
/// \param spritesheet Pointer to gl_LSpritesheet
/// \param new_clip New clipping rectangle to get sprite inside spritesheet
/// \return Index for newly added clipping sprite
///
extern int gl_LSpritesheet_add_clipsprite(gl_LSpritesheet* spritesheet, const LRect* new_clip);

///
/// Get clipping from specified index.
///
/// \param spritesheet Pointer to gl_LSpritesheet
/// \param index Index to get clipping rectangle
/// \return Clipping rectangle as LRect for specified index
///
extern LRect gl_LSpritesheet_get_clip(gl_LSpritesheet* spritesheet, int index);

///
/// Generate data buffer preparing for rendering.
///
/// \param spritesheet Pointer to gl_LSpritesheet
/// \return True if successfully generated, otherwise return false.
///
extern bool gl_LSpritesheet_generate_databuffer(gl_LSpritesheet* spritesheet);

///
/// Free VBO, IBO and all clipping array that used in rendering by the sheet.
///
/// \param spriteshet Pointer to gl_LSpritesheet
///
extern void gl_LSpritesheet_free_sheet(gl_LSpritesheet* spritesheet);

///
/// Render sprite from specified index.
///
/// \param spritesheet Pointer to gl_LSpritesheet
/// \param index Index representing sprite to render
/// \param x X position to render
/// \param y Y position to render
///
extern void gl_LSpritesheet_render_sprite(gl_LSpritesheet* spritesheet, int index, GLfloat x, GLfloat y);

#endif
