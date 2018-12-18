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

void gl_LTexture_render(gl_LTexture* texture, GLfloat x, GLfloat y, LFRect* clip)
{
  // texture coordinates
  GLfloat tex_top = 0.f;
  GLfloat tex_bottom = 1.f;
  GLfloat tex_left = 0.f;
  GLfloat tex_right = 1.f;

  // vertex coordinates
  GLfloat quad_width = texture->width;
  GLfloat quad_height = texture->height;

  // handle clipping
  if (clip != NULL)
  {
    // modify texture coordinates
    tex_left = clip->x / texture->width;
    tex_right = (clip->x + clip->w) / texture->width;
    tex_top = clip->y / texture->height;
    tex_bottom = (clip->y + clip->h) / texture->height;

    // modify vertex coordinates
    quad_width = clip->w;
    quad_height = clip->h;
  }

  // not to mess with matrix from outside
  // but use its matrix to operate on this further
  glPushMatrix();

  // move to rendering position
  glTranslatef(x, y, 0.f);

  // set texture id
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);

  // render texture quad
  glBegin(GL_QUADS);
    glTexCoord2f(tex_left, tex_top); glVertex2f(0.f, 0.f);
    glTexCoord2f(tex_right, tex_top); glVertex2f(quad_width, 0.f);
    glTexCoord2f(tex_right, tex_bottom); glVertex2f(quad_width, quad_height);
    glTexCoord2f(tex_left, tex_bottom); glVertex2f(0.f, quad_height);
  glEnd();

  // back to original matrix
  glPopMatrix();
}
