# Changes from original

* Fixed condition checking when to free `gl_LTexture`; mistake from using `&` for `&&`.
* Fixed duplicated call to free internal of `gl_LTexture` when using `gl_LTexture_load_texture_from_file` function.
* Fixed to set proper pixel format in readin and writing back. As follows
* Refactored and cleaned up slightly for DDS file loader function in `gl_LTexture.c`

    * `glGetTexImage()` with `GL_RGBA` will return pixel data in ARGB
    * `glTexSubImage2D()` with `GL_RGBA` will need us to set pixel data in ARGB
* Tested with different other .png files as well, including .dds (dxt5) file. Latter with no pixel manipulation.
* Added comment clearly over function in `gl_LTexture.h` that some of functions only work with RGBA file.

# Note

* Note the pixel data arrangement for reading and setting in `usercode_loadmedia()` function to understand how it relates to pixel format as read/write via `gl_LTexture`.
