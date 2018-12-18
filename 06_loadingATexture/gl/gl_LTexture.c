#include "gl_LTexture.h"
#include <stdlib.h>
#include "SDL_log.h"
#include "SDL_image.h"

static void init_defaults(gl_LTexture* texture)
{
  texture->texture_id = 0;
  texture->width = 0;
  texture->height = 0;
}

static void free_internal_texture(gl_LTexture* texture)
{
  if (texture != NULL & texture->texture_id != 0)
  {
    glDeleteTextures(1, &texture->texture_id);
    texture->texture_id = 0;
  }

  texture->width = 0;
  texture->height = 0;
}

gl_LTexture* gl_LTexture_new()
{
  gl_LTexture* out = malloc(sizeof(gl_LTexture));
  init_defaults(out);
  return out;
}

void gl_LTexture_free(gl_LTexture* texture)
{
  if (texture != NULL)
  {
    // free internal texture
    free_internal_texture(texture);
    // free allocated memory
    free(texture);
    texture = NULL;
  }
}

bool gl_LTexture_load_texture_from_file(gl_LTexture* texture, const char* path)
{
  SDL_Surface* loaded_surface = IMG_Load(path);
  if (loaded_surface == NULL)
  {
    SDL_Log("Unable to load image %s! SDL_Image error: %s", path, IMG_GetError());
    return false;
  }

  // set pixel data to texture
  if (!gl_LTexture_load_texture_from_pixels32(texture, loaded_surface->pixels, loaded_surface->w, loaded_surface->h))
  {
    SDL_Log("Failed to set pixel data to texture");
    return false;
  }

  // free surface
  SDL_FreeSurface(loaded_surface);
  loaded_surface = NULL;

  return true;
}

bool gl_LTexture_load_texture_from_pixels32(gl_LTexture* texture, GLuint* pixels, GLuint width, GLuint height)
{
  // free existing texture first if it exists
  // because user can load texture from pixels data multiple times
  free_internal_texture(texture);

  // get texture dimensions
  texture->width = width;
  texture->height = height;

  // generate texture id
  glGenTextures(1, &texture->texture_id);

  // bind texture id
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);

  // generate texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error loading texture from %p pixels! %s", pixels, gluErrorString(error));
    return false;
  }

  return true;
}

void gl_LTexture_render(gl_LTexture* texture, GLfloat x, GLfloat y)
{
  // not to mess with matrix from outside
  // but use its matrix to operate on this further
  glPushMatrix();

  // move to rendering position
  glTranslatef(x, y, 0.f);

  // set texture id
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);

  // render texture quad
  glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 0.f);
    glTexCoord2f(1.f, 0.f); glVertex2f(texture->width, 0.f);
    glTexCoord2f(1.f, 1.f); glVertex2f(texture->width, texture->height);
    glTexCoord2f(0.f, 1.f); glVertex2f(0.f, texture->height);
  glEnd();

  // back to original matrix
  glPopMatrix();
}
