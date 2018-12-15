///
/// User can implement their own user's code for game logic and rendering in this file.
/// It can include either OpenGL and normal SDL2 stuff.
///

#include "SDL.h"
#include "gl/glLOpenGL.h"
#include <stdbool.h>

///
/// Initialize
///
/// User should call this function to initialize with screen's dimension
/// as created externally first.
///
/// \param screen_width Screen width in pixel
/// \param screen_height Screen height in pixel
/// \return True if initialize successfully, otherwise return false.
///
extern bool usercode_init(int screen_width, int screen_height);

///
/// Update
///
/// \param delta_time Elapsed time since last frame in ms.
///
extern void usercode_update(float delta_time);

///
/// Render
///
extern void usercode_render();

///
/// Handle event.
///
/// \param e SDL_Event for incoming event
/// \param delta_time Elapsed time since last frame in ms.
///
extern void usercode_handle_event(SDL_Event *e, float delta_time);

///
/// Free all resource
///
extern void usercode_close();
