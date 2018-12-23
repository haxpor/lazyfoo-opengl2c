# Changes from original

* Not use Devilu for image operation ie. padding image etc. But do it manually.
* Fixed pixel bleeeding when render sub-region texture, refer to this [solution](https://stackoverflow.com/a/6051557/571227)
* Naming of unpadded image's dimension attribute inside struct is not the same as in original. It's `physical_width_` or `physical_height_` instead. So `width` and `height` are the expected original width or height of the texture itself before padding (if needed).
