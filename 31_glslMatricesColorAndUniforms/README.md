# Notes

* Also added rendering code via fixed pipeline showing side by side in the result screen.
* Framerate won't be drew because even after we use `glUseProgram()` to bind shader program, we can't go back to fixed function to render anymore. It's undefined behavior as see in manpage of such function.

