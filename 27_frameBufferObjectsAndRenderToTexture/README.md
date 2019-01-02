# Notes

* Added code rendering full screen at the background (no scale) to demonstrate how to render to such area when going full screen. This has benefit when we want to render decoration art work for wide-screen monitor.
* Added deleting of framebuffer name when done using it in `usercode_close()`.
* Taken into account fullscreen and windowed mode to render properly into frame buffer texture.
