#ifndef gl_LTexture_h_
#define gl_LTexture_h_

#include <stdbool.h>
#include "glLOpenGL.h"
#include "gl_types.h"

/// global shared variable that all instance of gl_LTexture will use
struct gl_ltextured_polygon_program2d_;
extern struct gl_ltextured_polygon_program2d_* shared_textured_shaderprogram;

typedef struct 
{
  /// texture id
  GLuint texture_id;

  /// texture width
  /// it's real width used for this texture if texture is not POT (power-of-two)
  int width;

  /// texture height
  /// it's real width used for this texture if texture is not POT (power-of-two)
  int height;

  /// pixels data of 32-bit in RBGA  of this texture
  GLuint* pixels;
  /// pixels data of 8-bit of this texture
  GLubyte* pixels8;

  // pixel format
  GLuint pixel_format;

  /// (read-only)
  /// real physical texture width in memory
  /// note: if texture is not POT then value will be different from 'width' as it will be in POT
  /// Internal system will determine next POT to assign to width and this is what 'physical_width_' is in
  /// case of NPOT texture
  int physical_width_;

  /// (read-only)
  /// real physical texture height in memory
  /// note: if texture is not POT then value will be different from 'height' as it will be in POT
  /// Internal system will determine next POT to assign to width and this is what 'physical_width_' is in
  /// case of NPOT texture
  int physical_height_;

  // VBO
  GLuint VBO_id;

  // IBO
  GLuint IBO_id;
} gl_LTexture;

///
/// set shared shader to use for entire gl_LTexture until set a new one.
///
/// \param shader pointer to gl_ltextured_polygon_program2d_
///
extern void gl_LTexture_set_shared_shader(struct gl_ltextured_polygon_program2d_* shader);

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
/// Load texture with extra parameters
/// Currently only load in format of RGBA8 for now.
///
/// \param texture Pointer to gl_LTexture
/// \param path Path to texture file to load
/// \param color_key Packed color key value in RGBA. This is a target color to replace it with transparent color in texture.
/// \return True if load successfully, otherwise return false.
///
extern bool gl_LTexture_load_texture_from_file_ex(gl_LTexture* texture, const char* path, GLuint color_key);

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
extern void gl_LTexture_render(gl_LTexture* texture, GLfloat x, GLfloat y, const LRect* clip);

///
/// Lock texture to manipulate pixel data.
/// Make sure your texture is in RGBA8 format. If it is not, then this will change your
/// image's format unexpectedly.
///
/// \param texture Pointer to gl_LTexture
/// \return True if lock successfully, otherwise return false.
///
extern bool gl_LTexture_lock(gl_LTexture* texture);

///
/// Unlock texture, pushing back manipulated pixel data back to GPU.
/// Make sure your texture is in RGBA8 format. If it is not, then this will change your
/// image's format unexpectedly.
///
/// \param texture Pointer to gl_LTexture
/// \return True if unlock successfully, otherwise return false.
///
extern bool gl_LTexture_unlock(gl_LTexture* texture);

///
/// Set pixel data at position x,y.
/// Only for RGBA8 pixel format.
///
/// \param texture Pointer to gl_LTexture
/// \param x X position (index-based) to set pixel value at
/// \param y Y position (index-based) to set pixel value at
/// \param value Pixel value to set. It's packed in 32 bit value of RGBA.
///
extern void gl_LTexture_set_pixel32(gl_LTexture* texture, GLuint x, GLuint y, GLuint pixel);

///
/// Get pixel value at position x,y.
/// Only for RGBA8 format.
///
/// \param texture Pointer to gl_LTexture
/// \param x Position x (index-based) to get pixel value from
/// \param y Position y (index-baesd) to get pixel value from
/// \return Pixel value packed as format RGBA8888
extern GLuint gl_LTexture_get_pixel32(gl_LTexture* texture, GLuint x, GLuint y);

///
/// Set pixel data at position x,y.
/// Only for 8-bit pixel format.
///
/// \param texture Pointer to gl_LTexture
/// \param x X position (index-based) to set pixel value at
/// \param y Y position (index-based) to set pixel value at
/// \param value Pixel value to set. It's a byte color value.
///
extern void gl_LTexture_set_pixel8(gl_LTexture* texture, GLuint x, GLuint y, GLubyte pixel);

///
/// Get pixel value at position x,y.
/// Only for 8-bit format.
///
/// \param texture Pointer to gl_LTexture
/// \param x Position x (index-based) to get pixel value from
/// \param y Position y (index-based) to get pixel value from
/// \return Pixel value as byte color.
///
extern GLubyte gl_LTexture_get_pixel8(gl_LTexture* texture, GLuint x, GLuint y);

///
/// Create blank canvas pixel space in format of RGBA 32-bit image.
///
/// \param texture Pointer to gl_LTexture
/// \param image_width Image width for canvas space to create
/// \param image_height Image height for canvas space to create
///
extern void gl_LTexture_create_pixels32(gl_LTexture* texture, GLuint image_width, GLuint image_height);

///
/// Copy pixels into texture
/// This function will free previous texture data if any, then recreate it again.
/// Texture has no need to be pre-created in blank.
///
/// \param texture Pointer to gl_LTexture
/// \param pixels Source pixels to copy into texture
/// \param image_width Image width of source pixels
/// \param image_height Image height of source pixels
///
extern void gl_LTexture_copy_pixels32(gl_LTexture* texture, GLuint* pixels, GLuint image_widtth, GLuint image_height);

///
/// Pad texture's pixels data to be in power-of-two (POT) dimensions.
///
/// \param texture Pointer to gl_LTexture
///
extern void gl_LTexture_pad_pixels32(gl_LTexture* texture);

///
/// Create blank canvas of pixels space in fromat of 8-bit grayscale color.
///
/// \param texture Pointer to gl_LTexture
/// \param image_width Image width for canvas space to create
/// \param image_height Image height for canvas space to create
///
extern void gl_LTexture_create_pixels8(gl_LTexture* texture, GLuint image_width, GLuint imgae_height);

///
/// Copy pixels (in 8-bit grayscale format) into texture.
/// This function will free previous texture data if any, then recreate it again.
///
/// \param texture Pointer to gl_LTexture
/// \param pixels Source pixels to copy into texture. It should be in 8-bit grayscale format.
/// \param image_width Image width of source pixels
/// \param image_height Image height of source pixels
///
extern void gl_LTexture_copy_pixels8(gl_LTexture* texture, GLubyte* pixels, GLuint image_width, GLuint image_height);

///
/// Pad texture's pixels data to be in power-of-two (POT) dimensions.
///
/// \param Pointer to texture
///
extern void gl_LTexture_pad_pixels8(gl_LTexture* texture);

///
/// Copy source pixels into destination texture at position x, and y.
/// Required that both texture and dst_texture are loaded with pixel data.
///
/// \param texture Pointer to gl_LTexture of source texture to copy pixels from
/// \param dst_x Destination x of destination texture to place source pixels at
/// \param dst_y Destination y of destination texture to place source pixels at
/// \parm dst_texture Destination texture to place pixels
///
extern void gl_LTexture_blit_pixels32(gl_LTexture* texture, GLuint dst_x, GLuint dst_y, const gl_LTexture* dst_texture);

///
/// Copy source pixels into destination texture at position x, and y.
/// This function works with 8-bit grayscale image format.
/// Required that both texture and dst_texture are loaded with pixel data.
///
/// \param texture Pointer to gl_LTexture of source texture to copy pixels from
/// \param dst_x Destination x of destination texture to place source pixels at
/// \param dst_y Destination y of destination texture to place source pixels at
/// \param dst_texture Destination texture to place pixels
///
extern void gl_LTexture_blit_pixels8(gl_LTexture* texture, GLuint dst_x, GLuint dst_y, const gl_LTexture* dst_texture);

#endif
