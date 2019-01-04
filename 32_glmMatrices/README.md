# Notes

* Use [cglm](https://github.com/recp/cglm) instead of glm which is only for C++.
* Migrated `gl_util_adapt_to_normal()` and `gl_util_adapt_to_letterbox()` from fixed pipeline to programmable pipeline.
* Added event function of `usercode_app_went_windowed_mode()` and `usercode_app_went_fullscreen()` when user switches between fullscreen and windowed mode. This is mainly used to re-calculate projection, and modelview matrix then update only the former to shader. Calculating modelview matrix is to save a few matrix operation especially scaling for fullscreen's dimensions.

# Changes from original

* Improved `gl_LShaderProgram_bind()` to check for existing already bound program and not attempt to re-bind it. This helps to not push burden in code management before calling such function whenever user wants as seen in `usercode_app_went_windowed_mode()` and `usercode_app_went_fullscreen()`.
