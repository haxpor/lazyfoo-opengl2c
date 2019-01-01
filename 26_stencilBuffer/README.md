# Notes

* Whenever we need to use stencil buffer with SDL2, we need to set its bit size via `SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);` before creating a new window. Otherwise, stencil won't work.
* Provide either triangle or rectangle polygon drawn onto stencil buffer, use w key to cycle between the twos.
