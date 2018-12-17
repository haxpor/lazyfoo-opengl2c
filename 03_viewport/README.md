# Changes from original

* Listen to `on_window_resize` event of `LWindow`
* Modified `LWindow` to expose ability to set fullscreen / windowed mode of window with coordinate changed to newer resolution (for now)
* Took into account multi-resolution whenever program runs on fullscreen mode (press Enter to toggle). It will apply letterbox appropriately.

