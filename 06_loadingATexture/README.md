# Changes from original

* Instead of using DevIL, we use `SDL2_image` to load image data then feed array of pixels result to create texture in opengl via `gl_LTexture_load_texture_from_pixels32()`.

