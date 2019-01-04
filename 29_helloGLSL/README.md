# Notes

* Fixed Makefile to be aware of internals header file for some of structs.
* Improved Makefile to not be redundancy in defining targets file in linking again
* Framerate won't be drew because even after we use `glUseProgram()` to bind shader program, we can't go back to fixed function to render anymore. It's undefined behavior as see in manpage of such function.

# Changes from original

* Define shader's source by using multiple-line string literals declaration in C.
