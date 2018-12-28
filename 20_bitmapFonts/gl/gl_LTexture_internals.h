#ifndef gl_LTexture_internals_h_
#define gl_LTexture_internals_h_

#include "gl_LTexture.h"
#include <stdbool.h>

/// these API meant to be used internally by library only (not limited to only gl_LTexture.c), not expose to user.
/// yeah technically, users can use if include this header, but you need to know what you're doing with these APIs.
/// Implementation source file is in gl_LTexture.c.

///
/// Free internal texture.
///
/// \param texture Pointer to gl_LTexture
///
extern void gl_LTexture_free_internal_texture(gl_LTexture* texture);

///
/// Load pixels data from file and set such pixel data into input gl_LTexture
///
/// \param texture Pointer to gl_LTexture
/// \param path Image path to load pixels
/// \return True if successfully load, otherwise return false
///
extern bool gl_LTexture_load_pixels_from_file(gl_LTexture* texture, const char* path);

///
/// Load texture from pre-created pixels (via gl_LTexture_load_pixels_from_file())
/// This function will create texture from loaded pixels as already set into gl_LTexture internally.
///
/// \parama texture Pointer to gl_LTexture
/// \return True if successfully load, otherwise return false.
///
extern bool gl_LTexture_load_texture_from_precreated_pixels32(gl_LTexture* texture);

#endif
