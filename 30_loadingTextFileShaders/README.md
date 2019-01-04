# Notes

* We place shader source files at `res/shaders` directory.
* Framerate won't be drew because even after we use `glUseProgram()` to bind shader program, we can't go back to fixed function to render anymore. It's undefined behavior as see in manpage of such function.
