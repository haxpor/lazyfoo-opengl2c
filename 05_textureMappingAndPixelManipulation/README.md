# Changes from original

* Don't expect the 100% port of code, I don't do that. Modified for learning purpose, proof-of-concept, and for better.
* Properly push matrix into stack first in render function of `gl_LTexture_render()` function to not mess with matrix used outside the call
* we have no need to center the screen by default as previous samples did, so positioning checkerboard at the center taken into correct effect in this case.
* added `usercode_loadmedia()` function and called from main source file to load media as need in user's code.
* use `GL_NEAREST` as mag and min filter of texture. This gives better result as we don't want to smooth things out.
