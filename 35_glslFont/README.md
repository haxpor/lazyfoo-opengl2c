# Changes

* Remove `glEnable(GL_TEXTURE_2D)` as this is non modern opengl function. If not, it will shout out error.
* Fixed `l_textured_polygon_program2d.frag` to have default texture color as opaque white so user has no need to set color every time.
