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
  GLuint width;
  /// texture height
  GLuint height;
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
