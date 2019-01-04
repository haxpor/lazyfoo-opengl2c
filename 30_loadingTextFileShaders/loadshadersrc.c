/**
 * 30 - Loading Text File Shaders
 */


#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "SDL.h"
#include "foundation/common.h"
#include "foundation/LTimer.h"
#include "foundation/LTexture.h"
#include "foundation/LWindow.h"
#include "gl/glLOpenGL.h"
#include "gl/gl_LTexture.h"
#include "gl/gl_LTexture_spritesheet.h"
#include "gl/gl_LFont.h"

#include "usercode.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define LOGICAL_WIDTH 640
#define LOGICAL_HEIGHT 480
#define SETFRAME(var, arg1, arg2, arg3, arg4)		\
  do {										\
    var.x = arg1;							\
    var.y = arg2;							\
    var.w = arg3;							\
    var.h = arg4;							\
  } while(0)

// cap thus using fixed deltaTime step
#define TARGET_FPS 60
#define FIXED_DELTATIME 1.0f / TARGET_FPS

// -- functions
bool init();
bool setup();
void handleEvent(SDL_Event *e, float deltaTime);
void update(float deltaTime);
void render(float deltaTime);
void close();

// opengl context
SDL_GLContext opengl_context;

// -- variables
bool quit = false;

// independent time loop
Uint32 currTime = 0;
Uint32 prevTime = 0;

#ifndef DISABLE_FPS_CALC
#define FPS_BUFFER 7+1
char fpsText[FPS_BUFFER];
gl_LFont* fps_font = NULL;
LSize fps_area = {SCREEN_WIDTH, SCREEN_HEIGHT};
#endif

// content's rect to clear color in render loop
SDL_Rect content_rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

bool init() {
  // initialize sdl
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
    return false;
  }

  // use opengl 2.1
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  // create window
  // if we set SDL_WINDOW_OPENGL flag then renderer won't be created for this window
  // thus make sure you cannot use LTexture anymore as it heavilty use renderer as created in LWindow
  gWindow = LWindow_new("30 - Loading Text File Shaders", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL, 0);
  if (gWindow == NULL) {
    SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
    return false;
  }
	// listen to window's resize event
	gWindow->on_window_resize = usercode_set_screen_dimension;

  // create opengl context
  opengl_context = SDL_GL_CreateContext(gWindow->window);
  if (opengl_context == NULL)
  {
    SDL_Log("OpenGL context could not be created: %s", SDL_GetError());
    return false;
  }

  // use vsync
  if (SDL_GL_SetSwapInterval(1) != 0)
  {
    SDL_Log("Warning: Unable to enable vsync! %s", SDL_GetError());
  }

  // init glew
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK)
  {
    SDL_Log("Failed initialize glew! %s", glewGetErrorString(glewError));
    return false;
  }

  // relay call to user's code in separate file
  if (!usercode_init(SCREEN_WIDTH, SCREEN_HEIGHT, LOGICAL_WIDTH, LOGICAL_HEIGHT))
  {
    SDL_Log("Failed to initialize user's code initializing function");
    return false;
  }

  // check opengl version we got
  printf("OpenGL version %s\nGLSL version: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

  // initialize png loading
  // see https://www.libsdl.org/projects/SDL_image/docs/SDL_image.html#SEC8
  // returned from IMG_Init is all flags initted, so we could check for all possible
  // flags we aim for
  int imgFlags = IMG_INIT_PNG;
  int inittedFlags = IMG_Init(imgFlags);
  if ( (inittedFlags & imgFlags) != imgFlags)
  {
    // from document, not always that error string from IMG_GetError() will be set
    // so don't depend on it, just for pure information
    SDL_Log("SDL_Image could not initialize! SDL_image Error: %s", IMG_GetError());
    return false;
  }

#ifndef DISABLE_SDL_TTF_LIB
  // initialize SDL_ttf
  if (TTF_Init() == -1)
  {
    SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s", TTF_GetError());
    return false;
  }
#endif

  return true;
}

// include any asset loading sequence, and preparation code here
bool setup()
{
#ifndef DISABLE_SDL_TTF_LIB
  // load font
  gFont = TTF_OpenFont("../Minecraft.ttf", 16);
  if (gFont == NULL)
  {
    SDL_Log("Failed to load Minecraft.ttf font: %s", TTF_GetError());
    return false;
  }
#endif

  // load font to render framerate
#ifndef DISABLE_FPS_CALC
  {
  gl_LTexture* raw_texture = gl_LTexture_new();
  gl_LSpritesheet* spritesheet = gl_LSpritesheet_new(raw_texture);

  fps_font = gl_LFont_new(spritesheet);
  if (!gl_LFont_load_freetype(fps_font, "../Minecraft.ttf", 14))
  {
    SDL_Log("Unable to load font for rendering framerate");
    return false;
  }
  }
#endif

  // load media from usercode
  if (!usercode_loadmedia())
  {
    SDL_Log("Failed to load media from usercode");
    return false;
  }

  return true;
}

void handleEvent(SDL_Event *e, float deltaTime)
{
  // user requests quit
  if (e->type == SDL_QUIT ||
      (e->key.keysym.sym == SDLK_ESCAPE))
  {
    quit = true;
  }
  else
  {
    // relay call to user's code in separate file
    usercode_handle_event(e, deltaTime);
  }
}

void update(float deltaTime)
{
  // relay call to user's code in separate file
  usercode_update(deltaTime);
}

void render(float deltaTime)
{
  if (!gWindow->is_minimized)
  {
    // relay call to user's code in separate file
    usercode_render();
  }

  // render frame rate on top of everything
#ifndef DISABLE_FPS_CALC
  // most likely rendering state is still ok, if you didn't weirdly modify
  // things before returning in usercode's render function, so we take benefit of it here
  snprintf(fpsText, FPS_BUFFER-1, "%d", (int)common_avgFPS);

  glColor3f(1.f, 1.f, 1.f);
  gl_LFont_render_textex(fps_font, fpsText, 0.f, 4.f, &fps_area, gl_LFont_TEXT_ALIGN_RIGHT | gl_LFont_TEXT_ALIGN_TOP);
#endif
}

void close()
{
  // relay call to user's code in separate file
  usercode_close();

#ifndef DISABLE_SDL_TTF_LIB
  // free font
  if (gFont != NULL)
  {
    TTF_CloseFont(gFont);
    gFont = NULL;
  }
#endif

#ifndef DISABLE_FPS_CALC
  if (fps_font != NULL)
    gl_LFont_free(fps_font);
#endif

  // destroy window
  LWindow_free(gWindow);

#ifndef DISABLE_SDL_TTF_LIB
  TTF_Quit();
#endif

  IMG_Quit();
  SDL_Quit();
}

int main(int argc, char* args[])
{
  // start up SDL and create window
  if (!init())
  {
    SDL_Log("Failed to initialize");
  }	
  else
  {
    // load media, and set up
    if (!setup())
    {
      SDL_Log("Failed to setup!");
    }
    else
    {
      // event handler
      SDL_Event e;

      // while application is running
      while (!quit)
      {
        // prepare delta time to feed to both handleEvent(), update(), and render()
        prevTime = currTime;
        currTime = SDL_GetTicks();
        // calculate per second
        float deltaTime = (currTime - prevTime) / 1000.0f;

        // fixed step
        common_frameTime += deltaTime;

#ifndef DISABLE_FPS_CALC
				// update accumulated time for calculating framerate
        common_frameAccumTime += deltaTime;
#endif
        if (common_frameTime >= FIXED_DELTATIME)
        {
#ifndef DISABLE_FPS_CALC
          common_frameCount++;

          // check to reset frame time
          if (common_frameAccumTime >= 1.0f)
          {
            common_avgFPS = common_frameCount / common_frameAccumTime;
            common_frameCount = 0;
            common_frameAccumTime -= 1.0f;
          }
#endif
          common_frameTime = 0.0f;

          // handle events on queue
          // if it's 0, then it has no pending event
          // we keep polling all event in each game loop until there is no more pending one left
          while (SDL_PollEvent(&e) != 0)
          {
            // update user's handleEvent()
            handleEvent(&e, FIXED_DELTATIME);
          }

          update(FIXED_DELTATIME);
          render(FIXED_DELTATIME);
        }
        else {
          render(0); 
        }

        // update screen
        SDL_GL_SwapWindow(gWindow->window);
      }
    }
  }

  // free resource and close SDL
  close();

  return 0;
}

