# Changes

* First sample in this repo to remove linking with `SDL2_TTF` as we directly use FreeType2. Configured using compile-time variable and not link against it in Makefile.
* Added 2 more .ttf files into project to test rendering at the middle at the bottom of the screen.
* Added a tiny spacing between characters when render on screen in `gl_LFont`'s rendering function.
