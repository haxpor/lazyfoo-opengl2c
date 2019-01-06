#include "gl_LTexture.h"
#include "gl_LTexture_internals.h"
#include "foundation/krr_math.h"
#include "gl/gl_util.h"
#include "gl/gl_ltextured_polygon_program2d.h"
#include "SDL_log.h"
#include "SDL_image.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct gl_ltextured_polygon_program2d_* shared_textured_shaderprogram = NULL;
static GLenum DEFAULT_TEXTURE_WRAP = GL_REPEAT;

// initialize defaults for texture
static void init_defaults(gl_LTexture* texture);
// find next POT value from input value
static int find_next_pot(int value);

// initialize VBO and IBO
static void init_VBO_IBO(gl_LTexture* texture);
static void free_VBO_IBO(gl_LTexture* texture);

void init_defaults(gl_LTexture* texture)
{
  texture->texture_id = 0;
  texture->width = 0;
  texture->height = 0;
  texture->pixels = NULL;
  texture->pixels8 = NULL;
  texture->pixel_format = 0;
  texture->physical_width_ = 0;
  texture->physical_height_ = 0;
  texture->VBO_id = 0;
  texture->IBO_id = 0;
}

void gl_LTexture_free_internal_texture(gl_LTexture* texture)
{
  if (texture != NULL && texture->texture_id != 0)
  {
    glDeleteTextures(1, &texture->texture_id);
    texture->texture_id = 0;
  }

  if (texture != NULL && texture->pixels != NULL)
  {
    free(texture->pixels);
    texture->pixels = NULL;
  }

  if (texture != NULL && texture->pixels8 != NULL)
  {
    free(texture->pixels8);
    texture->pixels8 = NULL;
  }

  texture->width = 0;
  texture->height = 0;
  texture->physical_width_ = 0;
  texture->physical_height_ = 0;
  texture->pixel_format = 0;

  free_VBO_IBO(texture);
}

void gl_LTexture_set_shared_shader(struct gl_ltextured_polygon_program2d_* shader)
{
  shared_textured_shaderprogram = shader;
}

gl_LTexture* gl_LTexture_new()
{
  gl_LTexture* out = malloc(sizeof(gl_LTexture));
  init_defaults(out);
  return out;
}

int find_next_pot(int value)
{
  // shift 1 bit to the left for input value, then 
  // logical AND with 1-most-siginicant-bit-mask to zero out all the less bits
  return (value << 1) & (1 << krr_math_bitcount(value));
}

struct DDS_PixelFormat;
struct DDS_Header;

static void _print_dds_header_struct(struct DDS_Header* header);

// struct represent dds format
struct DDS_PixelFormat {
  int size;
  int flags;
  int fourcc;
  int rgb_bitcount;
  int r_bitmask;
  int g_bitmask;
  int b_bitmask;
  int a_bitmask;
};

struct DDS_Header {
  int size;
  int flags;
  int height;
  int width;
  int pitch_or_linear_size;
  int depth;
  int mipmap_count;
  int reserved[11];
  struct DDS_PixelFormat dds_pixel_format;
  int caps;
  int caps2;
  int caps3;
  int caps4;
  int reserved2;
};

static void _print_dds_header_struct(struct DDS_Header* header)
{
  SDL_Log("DDS_Header");
  SDL_Log("- size of struct (should be 124): %d", header->size);
  SDL_Log("- flags: 0x%X", header->flags);
  SDL_Log("- height: %d", header->height);
  SDL_Log("- width: %d", header->width);
  SDL_Log("- pitch or linear size: %d", header->pitch_or_linear_size);
  SDL_Log("- depth (for volume texture): %d", header->depth);
  SDL_Log("- mipmap count: %d", header->mipmap_count);
  SDL_Log("- DDS_PixelFormat");
  SDL_Log("\t- size of struct (should be 32): %d", header->dds_pixel_format.size);
  SDL_Log("\t- flags: 0x%X", header->dds_pixel_format.flags);
  char fourcc_chrs[5];
  memset(fourcc_chrs, 0, sizeof(fourcc_chrs));
  strncpy(fourcc_chrs, (char*)&header->dds_pixel_format.fourcc, 4);
  SDL_Log("\t- fourCC: %s [0x%X]", fourcc_chrs, header->dds_pixel_format.fourcc);
  SDL_Log("\t- RGB bit count: %d", header->dds_pixel_format.rgb_bitcount);
  SDL_Log("\t- R bitmask: %d", header->dds_pixel_format.r_bitmask);
  SDL_Log("\t- G bitmask: %d", header->dds_pixel_format.g_bitmask);
  SDL_Log("\t- B bitmask: %d", header->dds_pixel_format.b_bitmask);
  SDL_Log("\t- A bitmask: %d", header->dds_pixel_format.a_bitmask);
}

void gl_LTexture_free(gl_LTexture* texture)
{
  if (texture != NULL)
  {
    // free internal texture
    gl_LTexture_free_internal_texture(texture);
    // free allocated memory
    free(texture);
    texture = NULL;
  }
}

bool gl_LTexture_load_texture_from_file(gl_LTexture* texture, const char* path)
{
  // free internal stuff will be done inside gl_LTexture_load_texture_from_pixels32() function

  SDL_Surface* loaded_surface = IMG_Load(path);
  if (loaded_surface == NULL)
  {
    SDL_Log("Unable to load image %s! SDL_Image error: %s", path, IMG_GetError());
    return false;
  }
  
  SDL_Log("format loaded surface: %s", SDL_GetPixelFormatName(loaded_surface->format->format));

  // convert pixel format
  SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_ABGR8888, 0);

  if (converted_surface == NULL)
  {
    SDL_Log("Cannot convert to ABGR8888 format");
    return false;
  }

  if (!gl_LTexture_load_texture_from_pixels32(texture, converted_surface->pixels, converted_surface->w, converted_surface->h))
  {
    SDL_Log("Failed to set pixel data to texture");
    return false;
  }

  SDL_Log("format: %s", SDL_GetPixelFormatName(converted_surface->format->format));

  // free surface
  SDL_FreeSurface(loaded_surface);
  SDL_FreeSurface(converted_surface);
  loaded_surface = NULL;
  converted_surface = NULL;

  return true;
}

bool gl_LTexture_load_texture_from_file_ex(gl_LTexture* texture, const char* path, GLuint color_key)
{
  // get pixels from file
  if (!gl_LTexture_load_pixels_from_file(texture, path))
  {
    SDL_Log("Failed to load pixels from file");
    return false;
  }

  // go through pixels to swap color as per color_key
  int pixel_count = texture->physical_width_ * texture->physical_height_;
  // map color key to ABGR format
  GLubyte* color_key_bytes = (GLubyte*)&color_key;
  // we get sequence of bytes which ranging from least to most significant so we can use it right away
  GLuint mapped_color_key = (color_key_bytes[0] << 24) | (color_key_bytes[1] << 16) | (color_key_bytes[2] << 8) | (color_key_bytes[3]);
  for (int i=0; i<pixel_count; i++)
  {
    // get pixel colors
    GLuint pixel = texture->pixels[i];
    if (pixel == mapped_color_key)
    {
      // make transparent (fully transparent white color)
      texture->pixels[i] = 0x00FFFFFF;
    }
  }

  // create a texture out of it
  if (!gl_LTexture_load_texture_from_precreated_pixels32(texture))
  {
    SDL_Log("Cannot create texture from pre-created pixels");
    return false;
  }

  return true;
}

bool gl_LTexture_load_dds_texture_from_file(gl_LTexture* texture, const char* path)
{
  // pre-check if user's system doesn't have required capability to load S3TC texture
  if (GLEW_EXT_texture_compression_s3tc == 0)
  {
    SDL_Log("S3TC texture not support for this system. Quit now");
    return false;
  }

  FILE* fp = NULL;
  fp = fopen(path, "r");
  if (fp == NULL)
  {
    SDL_Log("Unable to open file for read with errno: %d", errno);
    return false;
  }

  long total_size = 0;

  // find the total size of dds texture
  fseek(fp, 0, SEEK_END);
  total_size = ftell(fp);
  // reset file offset pointer back to start
  fseek(fp, 0, SEEK_SET);

  // ensure that total size is at least 124 + 4 to accomodate for
  // its magic number, and header
  if (total_size < 128)
  {
    SDL_Log("file might be corrupted or not recognized as DDS file format. It has less bytes that it should be.");
    fclose(fp);
    return false;
  }

  int f_nobj_read = 0;
  
  // 1st way to check for magic words of dds file - via integer
  int magic_number = 0;
  f_nobj_read = fread(&magic_number, 4, 1, fp);
  if (f_nobj_read != 1)
  {
    SDL_Log("Unable to read file (1st approach to read magic number)");
    fclose(fp);
    return false;
  }
  
  if (magic_number != 0x20534444)
  {
    SDL_Log("not dds file, found %d", magic_number);
    fclose(fp);
    return false;
  }

  SDL_Log("current file offset is at %ld", ftell(fp));

  // header section
  struct DDS_Header header;
  memset(&header, 0, sizeof(header));

  SDL_Log("size of header section for dds file format: %lu", sizeof(header));

  // read header section
  f_nobj_read = fread(&header, sizeof(header), 1, fp);
  if (f_nobj_read != 1)
  {
    SDL_Log("read head section error");
    fclose(fp);
    return false;
  }

  SDL_Log("---");

  // print struct info
  _print_dds_header_struct(&header);

  // check if texture is not square then we quit and report error
  if (header.width != header.height)
  {
    SDL_Log("Input texture %s is not square, stop reading further now.", path);
    fclose(fp);
    return false;
  }

  // check if sides are not power of two
  if (((header.width & (header.width - 1)) != 0) ||
      ((header.height & (header.height - 1)) != 0))
  {
    SDL_Log("Texture %s sides are not in power of two", path);
    fclose(fp);
    return false;
  }

  // read base image's pixel data
  // 0x31545844 represents "DXT1" in hexadecimal, we could convert fourcc to char* then compare to string literal as well
  SDL_Log("header.dds_pixel_format.fourcc: 0x%X", header.dds_pixel_format.fourcc);
  int blocksize = header.dds_pixel_format.fourcc == 0x31545844 ? 8 : 16;
  
  // set opengl format
  GLuint gl_format;
  if (blocksize == 8)
  {
    if ((header.dds_pixel_format.flags & 0x1) == 0)
    {
      gl_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      SDL_Log("RGB DXT1");
    }
    else
    {
      gl_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      SDL_Log("RGBA DXT1");
    }
  }
  // if blocksize is 16, then it has alpha channel thus we properly set opengl format
  // other formats not in switch case is ignored
  else 
  {
    switch (header.dds_pixel_format.fourcc)
    {
      // DXT3
      case 0x33545844:
        gl_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        SDL_Log("RGBA DXT3");
        break;
      // DXT5
      case 0x35545844:
        gl_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        SDL_Log("RGBA DXT5");
        break;
    }
  }

  SDL_Log("blocksize: %d", blocksize);

  // get total size of base image + mipmaps (if any)
  int images_size = ceil(header.width / 4.0) * ceil(header.height / 4.0) * blocksize;
  SDL_Log("level 0 width: %d, height: %d, size: %d", header.width, header.height, images_size);
  {
    int width = krr_math_max(1, header.width);
    int height = krr_math_max(1, header.height);
    int pre_width = width;
    int pre_height = height;

    for (int level=1; level<=header.mipmap_count; level++)
    {
      width = krr_math_max(1, width/2);
      height = krr_math_max(1, height/2);

      if (width == pre_width && height == pre_height)
      {
        // no need to proceed
        break;
      }
      else
      {
        // update previous width and height
        pre_width = width;
        pre_height = height;
      }

      int level_size = ceil(width / 4.0) * ceil(height / 4.0) * blocksize;
      SDL_Log("level %d width: %d, height: %d, size: %d", level, width, height, level_size);

      images_size += level_size;
    }
    SDL_Log("images_size: %d", images_size);
  }

  // define images buffer space
  unsigned char images_buffer[images_size];
  memset(images_buffer, 0, images_size);
  // read all image data into buffer
  f_nobj_read = fread(images_buffer, images_size, 1, fp);
  if (f_nobj_read != 1)
  {
    SDL_Log("read images data errno [EOF?:%d]", feof(fp));
    fclose(fp);
    return false;
  }
  // close file, we don't need reading from file anymore
  fclose(fp);
  fp = NULL;

  SDL_Log("---");

  // texture id
  GLuint texture_id;
  // generate texture id
  glGenTextures(1, &texture_id);
  // bind texture
  glBindTexture(GL_TEXTURE_2D, texture_id);

  // set texture paremters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, header.mipmap_count == 0 ? 0 : header.mipmap_count - 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_TEXTURE_WRAP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_TEXTURE_WRAP);

  SDL_Log("Format: 0x%X", gl_format);

  int offset = 0;
  int width = header.width;
  int height = header.height;

  for (int level=0; level<header.mipmap_count; level++)
  {
    // calculate size for this level of image
    int size  = ceil(width / 4.0) * ceil(height / 4.0) * blocksize;
    // create compressed texture
    glCompressedTexImage2D(GL_TEXTURE_2D, level, gl_format, width, height, 0, size, images_buffer + offset);

    SDL_Log("level %d, width: %d, height: %d, size: %d", level, width, height, size);

    // proceed next
    offset += size;
    // re-calculate size for mipmap
    width = krr_math_max(1, width/2);
    height = krr_math_max(1, height/2);
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  texture->texture_id = texture_id;
  texture->width = header.width;
  texture->height = header.height;
  texture->physical_width_ = header.width;
  texture->physical_height_ = header.height;

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error loading compressed texture %s", gl_util_error_string(error));
    return false;
  }

  // init VBO and IBO
  init_VBO_IBO(texture);

  // set pixel format
  texture->pixel_format = gl_format;

  return true;
}

bool gl_LTexture_load_texture_from_pixels32(gl_LTexture* texture, GLuint* pixels, GLuint width, GLuint height)
{
  // free existing texture first if it exists
  // because user can load texture from pixels data multiple times
  gl_LTexture_free_internal_texture(texture);

  bool is_need_to_resize = false;

  // check whether width is not POT
  if ((width & (width - 1)) != 0)
  {
    // find next POT for width
    texture->physical_width_ = find_next_pot(width);
    SDL_Log("physical_width: %u", texture->physical_width_);
    is_need_to_resize = true;
  }
  // otherwise width is the same as original input texture
  else
  {
    texture->physical_width_ = width;
  }

  // check whether height is not POT
  if ((height & (height - 1)) != 0)
  {
    // find next POT for height
    texture->physical_height_ = find_next_pot(height);
    SDL_Log("physical_height: %u", texture->physical_height_);
    is_need_to_resize = true;
  }
  // otherwise height is the same as original input texture
  else
  {
    texture->physical_height_ = height;
  }

  // get texture dimensions
  // these are the ones we gonna clip (if needed) to render finally
  texture->width = width;
  texture->height = height;

  SDL_Log("original width: %d", width);
  SDL_Log("original height: %d", height);

  // if need to resize, then put original pixels data at the top left
  // and pad the less with fully transparent white color
  GLuint* resized_pixels = NULL;
  if (is_need_to_resize)
  {
    // allocate 1D memory for resized pixels data
    resized_pixels = malloc(texture->physical_width_ * texture->physical_height_ * sizeof(GLuint));

    int offset = 0;

    // loop through all the pixels to set pixel data
    for (unsigned int x=0; x<texture->physical_width_; x++)
    {
      for (unsigned int y=0; y<texture->physical_height_; y++)
      {
        // calculate offset from 1D buffer
        offset = y * texture->physical_width_ + x;

        // if offset is in range to set pixel data from existing one
        // place existing pixels at the top left corner
        if (x >= 0 && x < texture->width &&
            y >= 0 && y < texture->height)
        {
          // calculate the offset for existing pixel data
          int existing_offset = y * texture->width + x;
          // set existing pixel data to final buffer we will use
          resized_pixels[offset] = pixels[existing_offset];
        }
        // if not then set it to fully transparent white color
        else
        {
          resized_pixels[offset] = (0 << 24) | (0 << 16) | (0 << 8) | 0;
        }
      }
    }
  }


  // generate texture id
  glGenTextures(1, &texture->texture_id);

  // bind texture id
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);

  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_TEXTURE_WRAP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_TEXTURE_WRAP);
  
  // there's no mipmap for this single texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

  // generate texture
  if (is_need_to_resize)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->physical_width_, texture->physical_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, resized_pixels);
  }
  else
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  }

  // unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);

  // free resized buffer (if need)
  if (is_need_to_resize)
  {
    free(resized_pixels);
    resized_pixels = NULL;
  }

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    SDL_Log("Error loading texture from %p pixels! %s", pixels, gl_util_error_string(error));
    return false;
  }

  // init VBO and IBO
  init_VBO_IBO(texture);

  // set pixel format
  texture->pixel_format = GL_RGBA;

  return true;
}

void gl_LTexture_render(gl_LTexture* texture, GLfloat x, GLfloat y, const LRect* clip)
{
  // texture coordinates
  // fixed pixel bleeding when we render sub-region of texture
  GLfloat tex_top = 0.0 + 0.5/texture->physical_height_;
  GLfloat tex_bottom = texture->height * 1.0 / texture->physical_height_ - 0.5/texture->physical_height_;
  GLfloat tex_left = 0.0 + 0.5/texture->physical_width_;
  GLfloat tex_right = texture->width * 1.0 / texture->physical_width_ - 0.5/texture->physical_width_;

  // vertex coordinates
  GLfloat quad_width = texture->width;
  GLfloat quad_height = texture->height;

  // handle clipping
  if (clip != NULL)
  {
    // modify texture coordinates
    tex_left = clip->x / texture->physical_width_ + 0.5/texture->physical_width_;
    tex_right = (clip->x + clip->w) / texture->physical_width_ - 0.5/texture->physical_width_;
    tex_top = clip->y / texture->physical_height_ + 0.5/texture->physical_height_;
    tex_bottom = (clip->y + clip->h) / texture->physical_height_ - 0.5/texture->physical_height_;

    // modify vertex coordinates
    quad_width = clip->w;
    quad_height = clip->h;
  }

  // move to rendering position
  glm_translate(shared_textured_shaderprogram->modelview_matrix, (vec3){x, y, 0.f});
  gl_ltextured_polygon_program2d_update_modelview_matrix(shared_textured_shaderprogram);

  // set vertex data
  LVertexData2D vertex_data[4];

  // texture coordinates
  vertex_data[0].texcoord.s = tex_left;     vertex_data[0].texcoord.t = tex_top;
  vertex_data[1].texcoord.s = tex_right;    vertex_data[1].texcoord.t = tex_top;
  vertex_data[2].texcoord.s = tex_right;    vertex_data[2].texcoord.t = tex_bottom;
  vertex_data[3].texcoord.s = tex_left;     vertex_data[3].texcoord.t = tex_bottom;

  // vertex position
  vertex_data[0].position.x = 0.f;          vertex_data[0].position.y = 0.f;
  vertex_data[1].position.x = quad_width;   vertex_data[1].position.y = 0.f;
  vertex_data[2].position.x = quad_width;   vertex_data[2].position.y = quad_height;
  vertex_data[3].position.x = 0.f;          vertex_data[3].position.y = quad_height;

  // set texture id
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);
  
  // enable vertex and texture coordinate vertex attribute arrays
  gl_ltextured_polygon_program2d_enable_attrib_pointers(shared_textured_shaderprogram);
  
    // bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, texture->VBO_id);
    // update vertex buffer data to GPU
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(LVertexData2D), vertex_data);

    // set texture coordinate data
    gl_ltextured_polygon_program2d_set_texcoord_pointer(shared_textured_shaderprogram, sizeof(LVertexData2D), (const GLvoid*)offsetof(LVertexData2D, texcoord));
    // set vertex data
    gl_ltextured_polygon_program2d_set_vertex_pointer(shared_textured_shaderprogram, sizeof(LVertexData2D), (const GLvoid*)offsetof(LVertexData2D, position));

    // draw quad using vertex and index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture->IBO_id);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // disable vertex and texture coord attribute pointer
  gl_ltextured_polygon_program2d_disable_attrib_pointers(shared_textured_shaderprogram);
}

bool gl_LTexture_lock(gl_LTexture* texture)
{
  // if texture is not locked yet, and it exists
  if (texture->pixels == NULL && texture->pixels8 == NULL && texture->texture_id != 0)
  {
    // check whether which pixel format to work with
    GLuint size = 0;
    if (texture->pixel_format == GL_ALPHA)
    {
      // note: use real width/height of texture which are physical_* in this case
      size = texture->physical_width_ * texture->physical_height_ * sizeof(GLubyte);
    }
    // otherwise treat it with GL_RGBA
    else
    {
      size = texture->physical_width_ * texture->physical_height_ * sizeof(GLuint);
    }

    // allocate memory space
    texture->pixels = malloc(size);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);

    // get pixels
    glGetTexImage(GL_TEXTURE_2D, 0, texture->pixel_format, GL_UNSIGNED_BYTE, texture->pixels);

    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
  }

  return false;
}

bool gl_LTexture_unlock(gl_LTexture* texture)
{
  // if texture is locked, and texture exists
  // as we have 2 underlying pixel formats to work with, if either one is not null, then it's fine to proceed
  if ((texture->pixels != NULL || texture->pixels8 != NULL) && texture->texture_id != 0)
  {
    // bind current texture
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);

    // get proper pixel buffer from pixel format texture was created to
    void* pixels = texture->pixel_format == GL_RGBA ? (void*)texture->pixels : (void*)texture->pixels8;
    // update texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->physical_width_, texture->physical_height_, texture->pixel_format, GL_UNSIGNED_BYTE, pixels); 

    // delete pixel data
    if (texture->pixel_format == GL_ALPHA)
    {
      free(texture->pixels8);
      texture->pixels8 = NULL;
    }
    else
    {
      free(texture->pixels);
      texture->pixels = NULL;
    }

    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
  }

  return false;
}

void gl_LTexture_set_pixel32(gl_LTexture* texture, GLuint x, GLuint y, GLuint pixel)
{
  texture->pixels[y * texture->physical_width_ + x] = pixel;
}

GLuint gl_LTexture_get_pixel32(gl_LTexture* texture, GLuint x, GLuint y)
{
  return texture->pixels[y * texture->physical_width_ + x];
}

void gl_LTexture_set_pixel8(gl_LTexture* texture, GLuint x, GLuint y, GLubyte pixel)
{
  texture->pixels8[y * texture->physical_width_ + x] = pixel;
}

GLubyte gl_LTexture_get_pixel8(gl_LTexture* texture, GLuint x, GLuint y)
{
  return texture->pixels8[y * texture->physical_width_ + x];
}

bool gl_LTexture_load_pixels_from_file(gl_LTexture* texture, const char* path)
{
  // free existing texture first if it exists
  gl_LTexture_free_internal_texture(texture);

  // load surface, then convert to our proper format first
  SDL_Surface* loaded_surface = IMG_Load(path);
  if (loaded_surface == NULL)
  {
    SDL_Log("Unable to load image %s! SDL_Image error: %s", path, IMG_GetError());
    return false;
  }
  
  SDL_Log("format loaded surface: %s", SDL_GetPixelFormatName(loaded_surface->format->format));

  // convert pixel format
  SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_ABGR8888, 0);

  if (converted_surface == NULL)
  {
    SDL_Log("Cannot convert to ABGR8888 format");
    return false;
  }

  // free surface we don't need now
  SDL_FreeSurface(loaded_surface);
  loaded_surface = NULL;

  SDL_Log("format: %s", SDL_GetPixelFormatName(converted_surface->format->format));

  // check whether we need to resize to POT texture
  bool is_need_to_resize = false;

  int width = converted_surface->w;
  int height = converted_surface->h;

  // check whether width is not POT
  if ((width & (width - 1)) != 0)
  {
    // find next POT for width
    texture->physical_width_ = find_next_pot(width);
    SDL_Log("physical_width: %u", texture->physical_width_);
    is_need_to_resize = true;
  }
  // otherwise width is the same as original input texture
  else
  {
    texture->physical_width_ = width;
  }

  // check whether height is not POT
  if ((height & (height - 1)) != 0)
  {
    // find next POT for height
    texture->physical_height_ = find_next_pot(height);
    SDL_Log("physical_height: %u", texture->physical_height_);
    is_need_to_resize = true;
  }
  // otherwise height is the same as original input texture
  else
  {
    texture->physical_height_ = height;
  }

  // get texture dimensions
  // these are the ones we gonna clip (if needed) to render finally
  texture->width = width;
  texture->height = height;

  SDL_Log("original width: %d", width);
  SDL_Log("original height: %d", height);

  // if need to resize, then put original pixels data at the top left
  // and pad the less with fully transparent white color
  GLuint* resized_pixels = NULL;
  if (is_need_to_resize)
  {
    // allocate 1D memory for resized pixels data
    resized_pixels = malloc(texture->physical_width_ * texture->physical_height_ * sizeof(GLuint));

    int offset = 0;

    // convert type of underlying pixels in surface to known type
    GLuint* surface_pixels = (GLuint*)converted_surface->pixels;

    // loop through all the pixels to set pixel data
    for (unsigned int x=0; x<texture->physical_width_; x++)
    {
      for (unsigned int y=0; y<texture->physical_height_; y++)
      {
        // calculate offset from 1D buffer
        offset = y * texture->physical_width_ + x;

        // if offset is in range to set pixel data from existing one
        // place existing pixels at the top left corner
        if (x >= 0 && x < texture->width &&
            y >= 0 && y < texture->height)
        {
          // calculate the offset for existing pixel data
          int existing_offset = y * texture->width + x;
          // set existing pixel data to final buffer we will use
          resized_pixels[offset] = surface_pixels[existing_offset];
        }
        // if not then set it to fully transparent white color
        else
        {
          resized_pixels[offset] = (0 << 24) | (0 << 16) | (0 << 8) | 0;
        }
      }
    }
  }

  // now we get pixels data ready
  if (is_need_to_resize)
  {
    // set pixel pointer to texture
    texture->pixels = resized_pixels;
  }
  else
  {
    const int size_bytes = converted_surface->w * converted_surface->h * sizeof(GLuint);
    // in case of no resizing, we need to copy pixel data
    // so it's safe to free its surface later
    GLuint* pixels_ptr = malloc(size_bytes);
    // copy pixel data
    // note: we need to copy as we will free converted surface soon after this
    memcpy(pixels_ptr, converted_surface->pixels, size_bytes);

    // set pixel pointer to texture
    texture->pixels = pixels_ptr;
  }

  // free surface
  SDL_FreeSurface(converted_surface);
  converted_surface = NULL;

  return true; 
}

bool gl_LTexture_load_pixels_from_file8(gl_LTexture* texture, const char* path)
{
  // free existing texture first if it exists
  gl_LTexture_free_internal_texture(texture);

  SDL_Surface* loaded_surface = IMG_Load(path);
  if (loaded_surface == NULL)
  {
    SDL_Log("Unable to load image %s! SDL_image error: %s", path, IMG_GetError());
    return false;
  }
  
  SDL_Log("format loaded surface: %s", SDL_GetPixelFormatName(loaded_surface->format->format));

  // check if the pixel format is not already in our interested format of BGR888
  Uint32 image_pixel_format = loaded_surface->format->format;
  if (image_pixel_format != SDL_PIXELFORMAT_BGR888)
  {
    SDL_Log("Need to convert to proper pixel format");

    // convert to more convenient format to work with
    SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_BGR888, 0);
    if (converted_surface == NULL)
    {
      SDL_Log("Cannot convert to BGR888 format");
      return false;
    }

    SDL_Log("converted format surface: %s", SDL_GetPixelFormatName(converted_surface->format->format));

    // free firstly loaded surface
    SDL_FreeSurface(loaded_surface);
    // set newly converted surface to loaded surface so we can proceed further
    loaded_surface = converted_surface;
  }

  // check whether we need to resize to POT texture
  bool is_need_to_resize = false;

  int width = loaded_surface->w;
  int height = loaded_surface->h;

  // check whether width is not POT
  if ((width & (width - 1)) != 0)
  {
    // find next POT for width
    texture->physical_width_ = find_next_pot(width);
    SDL_Log("physical_width: %u", texture->physical_width_);
    is_need_to_resize = true;
  }
  // otherwise width is the same as original input texture
  else
  {
    texture->physical_width_ = width;
  }

  // check whether height is not POT
  if ((height & (height - 1)) != 0)
  {
    // find next POT for height
    texture->physical_height_ = find_next_pot(height);
    SDL_Log("physical_height: %u", texture->physical_height_);
    is_need_to_resize = true;
  }
  // otherwise height is the same as original input texture
  else
  {
    texture->physical_height_ = height;
  }

  // get texture dimensions
  // these are the ones we gonna clip (if needed) to render finally
  texture->width = width;
  texture->height = height;

  SDL_Log("original width: %d", width);
  SDL_Log("original height: %d", height);

  // if need to resize, then put original pixels data at the top left
  // and pad the less with fully transparent white color
  // this is the final buffer to store 8-bit pixel data
  GLubyte* resized_pixels = NULL;
  if (is_need_to_resize)
  {
    // allocate 1D memory for resized pixels data
    resized_pixels = malloc(texture->physical_width_ * texture->physical_height_ * sizeof(GLubyte));

    int offset = 0;

    // convert type of underlying pixels in surface to known type
    GLuint* surface_pixels = (GLuint*)loaded_surface->pixels;

    // loop through all the pixels to set pixel data
    for (unsigned int x=0; x<texture->physical_width_; x++)
    {
      for (unsigned int y=0; y<texture->physical_height_; y++)
      {
        // calculate offset from 1D buffer
        offset = y * texture->physical_width_ + x;

        // if offset is in range to set pixel data from existing one
        // place existing pixels at the top left corner
        if (x >= 0 && x < texture->width &&
            y >= 0 && y < texture->height)
        {
          // calculate the offset for existing pixel data
          int existing_offset = y * texture->width + x;
          // set existing pixel data to final buffer we will use
          // only get red color component
          resized_pixels[offset] = surface_pixels[existing_offset] & 0xff;
        }
        // if not then set it to fully black color (no alpha)
        else
        {
          resized_pixels[offset] = 0x00;
        }
      }
    }
  }
  // only need to get relevant pixel data setting to final buffer
  else
  {
    // allocate 1D memory for resized pixels data
    resized_pixels = malloc(texture->physical_width_ * texture->physical_height_ * sizeof(GLubyte));

    int offset = 0;

    // convert type of underlying pixels in surface to known type
    GLuint* surface_pixels = (GLuint*)loaded_surface->pixels;

    // loop through all the pixels to set pixel data
    for (unsigned int x=0; x<texture->physical_width_; x++)
    {
      for (unsigned int y=0; y<texture->physical_height_; y++)
      {
        // calculate offset from 1D buffer
        offset = y * texture->physical_width_ + x;

        // calculate the offset for existing pixel data
        int existing_offset = y * texture->width + x;
        // set existing pixel data to final buffer we will use
        // only get red color component
        resized_pixels[offset] = surface_pixels[existing_offset] & 0xff;
      }
    }
  }

  // now we get pixels data ready
  // set pixel pointer to texture
  texture->pixels8 = resized_pixels;

  // free surface
  SDL_FreeSurface(loaded_surface);
  loaded_surface = NULL;

  return true;
}

bool gl_LTexture_load_texture_from_precreated_pixels32(gl_LTexture* texture)
{
  // if there's loaded pixels already set to texture
  // but texture is not created yet
  if (texture->pixels != NULL && texture->texture_id == 0)
  {
    // generate texture id
    glGenTextures(1, &texture->texture_id);

    // bind texture id
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_TEXTURE_WRAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_TEXTURE_WRAP);

    // there's no mipmap for this single texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    
    // generate texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->physical_width_, texture->physical_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);

    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      SDL_Log("Error loading texture from pixels [%p]: %s", texture->pixels, gl_util_error_string(error));
      return false;
    }
    else
    {
      // free allocated pixel data space
      free(texture->pixels);
      texture->pixels = NULL;

      // init VBO and IBO
      init_VBO_IBO(texture);

      // set pixel format
      texture->pixel_format = GL_RGBA;

      return true;
    }
  }
  else
  {
    if (texture->pixels == NULL)
    {
      SDL_Log("Cannot load texture from pre-created pixels, there is no pixels data");
    }

    if (texture->texture_id != 0)
    {
      SDL_Log("Cannot load texture from pre-created pixels, there was already existing texture");
    }
  }

  return false;
}

bool gl_LTexture_load_texture_from_precreated_pixels8(gl_LTexture* texture)
{
  // if there's loaded pixels already set to texture
  // but texture is not created yet
  if (texture->pixels8 != NULL && texture->texture_id == 0)
  {
    // generate texture id
    glGenTextures(1, &texture->texture_id);

    // bind texture id
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_TEXTURE_WRAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_TEXTURE_WRAP);

    // there's no mipmap for this single texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    
    // generate texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture->physical_width_, texture->physical_height_, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture->pixels8);

    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      SDL_Log("Error loading texture from pixels [%p]: %s", texture->pixels, gl_util_error_string(error));
      return false;
    }
    else
    {
      // free allocated pixel data space
      free(texture->pixels);
      texture->pixels = NULL;

      // init VBO and IBO
      init_VBO_IBO(texture);

      // set pixel format
      texture->pixel_format = GL_ALPHA;

      return true;
    }
  }
  else
  {
    if (texture->pixels == NULL)
    {
      SDL_Log("Cannot load texture from pre-created pixels8, there is no pixels data");
    }

    if (texture->texture_id != 0)
    {
      SDL_Log("Cannot load texture from pre-created pixels, there was already existing texture");
    }
  }

  return false;
}

void init_VBO_IBO(gl_LTexture* texture)
{
  // if texture is loaded and VBO doesn't already exist
  if (texture->texture_id != 0 && texture->VBO_id == 0)
  {
    // vertex data
    LVertexData2D vertex_data[4];
    GLuint index_data[4];

    // set rendering indices
    index_data[0] = 0;
    index_data[1] = 1;
    index_data[2] = 2;
    index_data[3] = 3;

    // create VBO
    glGenBuffers(1, &texture->VBO_id);
    glBindBuffer(GL_ARRAY_BUFFER, texture->VBO_id);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(LVertexData2D), vertex_data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // create IBO
    glGenBuffers(1, &texture->IBO_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture->IBO_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), index_data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
}

void free_VBO_IBO(gl_LTexture* texture)
{
  if (texture->VBO_id != 0 && texture->IBO_id != 0)
  {
    glDeleteBuffers(1, &texture->VBO_id);
    glDeleteBuffers(1, &texture->IBO_id);

    texture->VBO_id = 0;
    texture->IBO_id = 0;
  }
}

void gl_LTexture_create_pixels32(gl_LTexture* texture, GLuint image_width, GLuint image_height)
{
  // valid dimension
  if (image_width > 0 && image_height > 0)
  {
    // get rid of any current texture data
    gl_LTexture_free_internal_texture(texture);

    // create pixels
    GLuint pixel_count = image_width * image_height;
    texture->pixels = malloc(pixel_count * sizeof(GLuint));
    // set to all fully transparent black color
    memset(texture->pixels, 0, pixel_count * sizeof(GLuint));

    // copy pixel data
    texture->width = image_width;
    texture->height = image_height;
    // not just yet make it POT
    texture->physical_width_ = image_width;
    texture->physical_height_ = image_height;

    // set pixel format
    texture->pixel_format = GL_RGBA;
  }
}

void gl_LTexture_copy_pixels32(gl_LTexture* texture, GLuint* pixels, GLuint image_width, GLuint image_height)
{
  // pixels has valid dimensions
  if (image_width > 0 && image_height > 0)
  {
    // get rid of any current texture data
    gl_LTexture_free_internal_texture(texture);

    // copy pixels
    GLuint size = image_width * image_height * sizeof(GLuint);
    texture->pixels = malloc(size);
    memcpy(texture->pixels, pixels, size);

    // copy pixel data
    texture->width = image_width;
    texture->height = image_height;
    // not just yet make it POT
    texture->physical_width_ = image_width;
    texture->physical_height_ = image_height;

    // set pixel format
    texture->pixel_format = GL_RGBA;
  }
}

void gl_LTexture_pad_pixels32(gl_LTexture* texture)
{
  // if there are pixels to pad
  if (texture->pixels != NULL)
  {
    // old texture attributes
    // check whether we need to resize to POT texture
    bool is_need_to_resize = false;

    // get original width and height
    int width = texture->physical_width_;
    int height = texture->physical_height_;

    // check whether width is not POT
    if ((width & (width - 1)) != 0)
    {
      // find next POT for width
      texture->physical_width_ = find_next_pot(width);
      SDL_Log("physical_width: %u", texture->physical_width_);
      is_need_to_resize = true;
    }

    // check whether height is not POT
    if ((height & (height - 1)) != 0)
    {
      // find next POT for height
      texture->physical_height_ = find_next_pot(height);
      SDL_Log("physical_height: %u", texture->physical_height_);
      is_need_to_resize = true;
    }

    // if need to resize, then put original pixels data at the top left
    if (is_need_to_resize)
    {
      // stride size for each pixel
      GLuint stride = sizeof(GLuint);

      // allocate 1D memory for resized pixels data (POT texture)
      GLuint* resized_pixels = malloc(texture->physical_width_ * texture->physical_height_ * stride);

      // copy pixels data into resized pixels, place at the top left
      // copy row by row
      for (int row=0; row<texture->height; row++)
      {
        memcpy(resized_pixels + row*texture->physical_width_, texture->pixels + row*texture->width, texture->width * sizeof(GLuint));
      }

      // delete old pixels data
      free(texture->pixels);
      // set to new resized pixels
      texture->pixels = resized_pixels;
    }
  }
}

void gl_LTexture_blit_pixels32(gl_LTexture* texture, GLuint dst_x, GLuint dst_y, const gl_LTexture* dst_texture)
{
  // there are pixels to blit
  if (texture->pixels != NULL && dst_texture->pixels != NULL)
  {
    // copy pixels rows
    GLuint* dst_pixels = dst_texture->pixels;
    GLuint* src_pixels = texture->pixels;

    // note: we don't operate on void* thus no need to multiply by size of (stride) in destination or source pointer, but not size of bytes to copy
    for (int row=0; row<texture->height; row++)
    {
      memcpy(dst_pixels + (row+dst_y)*dst_texture->physical_width_ + dst_x, src_pixels + row*texture->physical_width_, texture->width*sizeof(GLuint));
    }
  }
}

void gl_LTexture_create_pixels8(gl_LTexture* texture, GLuint image_width, GLuint image_height)
{
  // valid dimensions
  if (image_width > 0 && image_height > 0)
  {
    // get rid of any current texture data
    gl_LTexture_free_internal_texture(texture);

    // create pixels
    GLuint pixel_count = image_width * image_height;
    texture->pixels8 = malloc(pixel_count * sizeof(GLubyte));
    // set to all fully transparent black color
    memset(texture->pixels8, 0, pixel_count * sizeof(GLubyte));

    // copy pixel data
    texture->width = image_width;
    texture->height = image_height;
    // not just yet make it POT
    texture->physical_width_ = image_width;
    texture->physical_height_ = image_height;

    // set pixel format
    texture->pixel_format = GL_ALPHA;
  }
}

void gl_LTexture_copy_pixels8(gl_LTexture* texture, GLubyte* pixels, GLuint image_width, GLuint image_height)
{
  // pixels has valid dimensions
  if (image_width > 0 && image_height > 0)
  {
    // get rid of any current texture data
    gl_LTexture_free_internal_texture(texture);

    // copy pixels
    GLuint size = image_width * image_height * sizeof(GLubyte);
    texture->pixels8 = malloc(size);
    memcpy(texture->pixels8, pixels, size);

    // copy pixel data
    texture->width = image_width;
    texture->height = image_height;
    // not just yet make it POT
    texture->physical_width_ = image_width;
    texture->physical_height_ = image_height;

    // set pixel format
    texture->pixel_format = GL_ALPHA;
  }
}

void gl_LTexture_pad_pixels8(gl_LTexture* texture)
{
  // if there are pixels to pad
  if (texture->pixels8 != NULL)
  {
    // old texture attributes
    // check whether we need to resize to POT texture
    bool is_need_to_resize = false;

    // get original width and height
    int width = texture->physical_width_;
    int height = texture->physical_height_;

    // check whether width is not POT
    if ((width & (width - 1)) != 0)
    {
      // find next POT for width
      texture->physical_width_ = find_next_pot(width);
      SDL_Log("physical_width: %u", texture->physical_width_);
      is_need_to_resize = true;
    }

    // check whether height is not POT
    if ((height & (height - 1)) != 0)
    {
      // find next POT for height
      texture->physical_height_ = find_next_pot(height);
      SDL_Log("physical_height: %u", texture->physical_height_);
      is_need_to_resize = true;
    }

    // if need to resize, then put original pixels data at the top left
    if (is_need_to_resize)
    {
      // stride size for each pixel
      GLuint stride = sizeof(GLubyte);

      // allocate 1D memory for resized pixels data (POT texture)
      GLubyte* resized_pixels = malloc(texture->physical_width_ * texture->physical_height_ * stride);

      // copy pixels data into resized pixels, place at the top left
      // copy row by row
      for (int row=0; row<texture->height; row++)
      {
        memcpy(resized_pixels + row*texture->physical_width_, texture->pixels8 + row*texture->width, texture->width * sizeof(GLubyte));
      }

      // delete old pixels data
      free(texture->pixels8);
      // set to new resized pixels
      texture->pixels8 = resized_pixels;
    }
  }
}

void gl_LTexture_blit_pixels8(gl_LTexture* texture, GLuint dst_x, GLuint dst_y, const gl_LTexture* dst_texture)
{
  // there are pixels to blit
  if (texture->pixels8 != NULL && dst_texture->pixels8 != NULL)
  {
    // copy pixels rows
    GLubyte* dst_pixels = dst_texture->pixels8;
    GLubyte* src_pixels = texture->pixels8;

    // note: we don't operate on void* thus no need to multiply by size of (stride) in destination or source pointer, but not size of bytes to copy
    for (int row=0; row<texture->height; row++)
    {
      memcpy(dst_pixels + (row+dst_y)*dst_texture->physical_width_ + dst_x, src_pixels + row*texture->physical_width_, texture->width*sizeof(GLubyte));
    }
  }
}
