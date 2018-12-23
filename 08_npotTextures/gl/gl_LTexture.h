#ifndef gl_LTexture_h_
#define gl_LTexture_h_

#include <stdbool.h>
#include "glLOpenGL.h"
#include "gl_types.h"

typedef struct 
{
  /// texture id
  GLuint texture_id;

  /// texture width
  /// it's real width used for this texture if texture is not POT (power-of-two)
  GLuint width;

  /// texture height
  /// it's real width used for this texture if texture is not POT (power-of-two)
  GLuint height;

  /// (internally use)
  /// real physical texture width in memory
  /// note: if texture is not POT then value will be different from 'width' as it will be in POT
  /// Internal system will determine next POT to assign to width and this is what 'physical_width_' is in
  /// case of NPOT texture
  GLuint physical_width_;

  /// (internally use)
  /// real physical texture height in memory
  /// note: if texture is not POT then value will be different from 'height' as it will be in POT
  /// Internal system will determine next POT to assign to width and this is what 'physical_width_' is in
  /// case of NPOT texture
  GLuint physical_height_;
} gl_LTexture;

///
/// Create a new texture.
///
/// \return Newly created gl_LTexture on heap.
///
extern gl_LTexture* gl_LTexture_new();

///
/// Free texture.
///
/// \param texture gl_LTexture to be freed.
///
extern void gl_LTexture_free(gl_LTexture* texture);

///
/// Load texture from file.
/// Currently only load in format of RGBA8888 for now.
/// Make sure input file format is in RGBA8888.
///
/// \param texture Pointer to gl_LTexture
/// \param path Path to texture file to load
/// \return True if load successfully, otherwise return false.
///
extern bool gl_LTexture_load_texture_from_file(gl_LTexture* texture, const char* path);

///
/// Load compressed texture, DDS from file.
/// Input file needs to be in .dds (DXT1, DXT3, or DXT5) format.
/// To be loaded texture needs to be in POT (power-of-two) texture, and square.
///
/// \param texture Pointer to gl_LTexture
/// \param path Path to texture file to load
/// \return True if load successfully, otherwise return false.
///
extern bool gl_LTexture_load_dds_texture_from_file(gl_LTexture* texture, const char* path);

///
/// Load pixels 32-bit (8 bit per pixel) data into texture.
///
/// \param texture gl_LTexture to load input pixels data into it.
/// \param pixels Pixels data; 32-bit, 8-bit per pixel
/// \param width Width of input pixels data
/// \param height Height of input pixels data
/// \return True if loading is successful, otherwise return false.
///
extern bool gl_LTexture_load_texture_from_pixels32(gl_LTexture* texture, GLuint* pixels, GLuint width, GLuint height);

///
/// Render texture.
///
/// \param texture gl_LTexture to render
/// \param x Position x to render
/// \param y Position y to render
/// \param clip Clipping rectangle to render part of the texture. NULL to render fully of texture.
///
extern void gl_LTexture_render(gl_LTexture* texture, GLfloat x, GLfloat y, LFRect* clip);

#endif
