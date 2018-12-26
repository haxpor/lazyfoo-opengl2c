# Changes from original

* Rename type struct related to GL to `L...` for all i.e. `LSize`, `LRect`, etc.
* Use `LTexCoord2D` instead of `LTexCoord` as there is texture coordinate in 3d as well.
* Always free VBO and IBO in `free_internal_texture`.
