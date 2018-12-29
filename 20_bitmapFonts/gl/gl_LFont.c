#include "gl_LFont.h"
#include <stdlib.h>
#include <stddef.h>
#include "SDL_log.h"
#include "foundation/vector.h"
#include "gl/gl_LTexture_internals.h"

static void init_defaults_(gl_LFont* font);
static void free_internals_(gl_LFont* font);

void init_defaults_(gl_LFont* font)
{
  font->spritesheet = NULL;
  font->space = 0.f;
  font->line_height = 0.f;
  font->newline = 0.f;
}

void free_internals_(gl_LFont* font)
{
  gl_LSpritesheet_free(font->spritesheet);

  font->space = 0.f;
  font->line_height = 0.f;
  font->newline = 0.f;
}

gl_LFont* gl_LFont_new(gl_LSpritesheet* spritesheet)
{
  gl_LFont *out = malloc(sizeof(gl_LFont));
  init_defaults_(out);

  // set spritesheet
  out->spritesheet = spritesheet;

  return out;
}

void gl_LFont_free(gl_LFont* font)
{
  free_internals_(font);

  free(font);
  font = NULL;
}

bool gl_LFont_load_bitmap(gl_LFont* font, const char* path)
{
  // expect image that is grayscale, in 16x16 ASCII order

  // we expect an image that has only black, white and shades of gray
  // it can be grayscale image in PNG file format
  const GLuint black_pixel = 0xFF000000;

  // get rid of the font if it exists
  gl_LFont_free_font(font);

  // image pixels loaded
  gl_LTexture* texture = font->spritesheet->ltexture;
  if (!gl_LTexture_load_pixels_from_file(texture, path))
  {
    SDL_Log("Unable to load pixels from file");
    return false;
  }

  // get cell dimensions
  // image has 16x16 cells
  GLfloat cell_width = texture->width / 16.f;
  GLfloat cell_height = texture->height / 16.f;

  // get letter top and bottom
  GLuint top = cell_height;
  GLuint bottom = 0.f;
  GLuint a_bottom = 0.f;

  // current pixel coordinate
  int p_x = 0;
  int p_y = 0;

  // base cell offsets
  int b_x = 0;
  int b_y = 0;

  // begin parsing bitmap font
  GLuint current_char = 0;
  LRect next_clip = { 0.f, 0.f, cell_width, cell_height };

  // go through cell rows
  for (int row = 0; row < 16; row++)
  {
    // go through each cell column in the row
    for (int col = 0; col < 16; col++)
    {
      // begin cell parsing
      // set base offsets
      b_x = cell_width * col;
      b_y = cell_height * row;
      
      // initialize clip
      next_clip.x = b_x;
      next_clip.y = b_y;

      next_clip.w = cell_width;
      next_clip.h = cell_height;

      // find boundary for character
      // left side
      for (int p_col = 0; p_col < cell_width; p_col++)
      {
        for (int p_row = 0; p_row < cell_height; p_row++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non-background pixel found
          if (gl_LTexture_get_pixel32(texture, p_x, p_y) != black_pixel)
          {
            // set sprite's x offset
            next_clip.x = p_x;

            // break the loop
            p_col = cell_width;
            p_row = cell_height;
          }
        }
      }

      // right side
      for (int p_col = cell_width-1; p_col >= 0; p_col--)
      {
        for (int p_row = 0; p_row < cell_height; p_row++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non background pixel found
          if (gl_LTexture_get_pixel32(texture, p_x, p_y) != black_pixel)
          {
            // set sprite's width
            next_clip.w = (p_x - next_clip.x) + 1;

            // break the loop
            p_col = -1;
            p_row = cell_height;
          }
        }
      }

      // find top
      for (int p_row = 0; p_row < cell_height; p_row++)
      {
        for (int p_col = 0; p_col < cell_width; p_col++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non background pixel found
          if (gl_LTexture_get_pixel32(texture, p_x, p_y) != black_pixel)
          {
            // new top found
            if (p_row < top)
            {
              top = p_row;
            }

            // break the loop
            p_row = cell_height;
            p_col = cell_width;
          }
        }
      }

      // find bottom
      for (int p_row = cell_height - 1; p_row >= 0; p_row--)
      {
        for (int p_col = 0; p_col < cell_width; p_col++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non background pixel found
          if (gl_LTexture_get_pixel32(texture, p_x, p_y) != black_pixel)
          {
            // set baseline
            if (current_char == 'A')
            {
              a_bottom = p_row;
            }

            // new bottom found
            if (p_row > bottom)
            {
              bottom = p_row;
            }

            // break the loop
            p_row = -1;
            p_col = cell_width;
          }
        }
      }

      // go to the next character
      vector_add(font->spritesheet->clips, &next_clip);
      current_char++;
    }
  }

  // set top
  // by lopping off extra height from all the character sprites
  vector* clips = font->spritesheet->clips;
  for (int t = 0; t < clips->len; t++)
  {
    // get LRect
    LRect* rect = (LRect*)vector_get(clips, t);

    // update back
    rect->y += top;
    rect->h -= top;
  }

  // blend
  // go through pixels
  const int pixel_count = texture->width * texture->height;
  // we can get this as we load pixels from file, it's directly set pixels data to
  // our texture for us
  GLuint* pixels = texture->pixels;
  for (int i=0; i<pixel_count; i++)
  {
    // get individual color components
    // this will get us ABGR automatically in the process
    // remember: if we don't get bytes via pointer like in the next line, but use bit operation, we have to properly do it to get in ABGR format
    GLubyte* colors = (GLubyte*)&pixels[i];

    // white pixel shaded by transparency
    colors[0] = 0xff;
    colors[1] = 0xff;
    colors[2] = 0xff;
    colors[3] = colors[4];
  }

  // create texture from manipulated pixels
  if (!gl_LTexture_load_texture_from_precreated_pixels32(texture))
  {
    SDL_Log("Unable to create texture from precreated pixels");
    return false;
  }

  // build vertex buffer from spritesheet data
  if (!gl_LSpritesheet_generate_databuffer(font->spritesheet))
  {
    SDL_Log("Unable to generate databuffer for spritesheet");
    return false;
  }

  // set texture wrap
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glBindTexture(GL_TEXTURE_2D, 0);

  // set spacing variables
  font->space = cell_width / 2.f;
  font->newline = a_bottom - top;
  font->line_height = bottom - top;

  return true;
}

void gl_LFont_free_font(gl_LFont* font)
{
  // clear the sheet
  gl_LSpritesheet_free_sheet(font->spritesheet);
  // clear the underlying 
  gl_LTexture_free_internal_texture(font->spritesheet->ltexture);

  // reinitialize spacing constants
  font->space = 0.f;
  font->line_height = 0.f;
  font->newline = 0.f;
}

void gl_LFont_render_text(gl_LFont* font, const char* text, GLfloat x, GLfloat y)
{
  // if there is texture to render from
  if (font->spritesheet->ltexture->texture_id != 0)
  {
    // get spritesheet
    gl_LSpritesheet* ss = font->spritesheet;
    // get texture id
    GLuint texture_id = ss->ltexture->texture_id;

    // draw position
    GLfloat render_x = x;
    GLfloat render_y = y;

    // don't mess with external matrix
    glPushMatrix();

    glTranslatef(x, y, 0.f);

    // set texture
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // enable vertex and texture coordinate arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // bind vertex data
    glBindBuffer(GL_ARRAY_BUFFER, ss->vertex_data_buffer);

    // set texture coordinate data
    glTexCoordPointer(2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof(LVertexData2D, texcoord));
    glVertexPointer(2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof(LVertexData2D, position));

    // go through string
    int text_length = strlen(text);
    for (int i=0; i<text_length; i++)
    {
      // space
      if (text[i] == ' ')
      {
        glTranslatef(font->space, 0.f, 0.f);
        render_x += font->space;
      }
      else if (text[i] == '\n')
      {
        glTranslatef(x - render_x, font->newline, 0.f);
        render_y += font->newline;
        render_x += x - render_x;
      }
      else
      {
        // get ascii
        GLuint ascii = (unsigned char)text[i];

        // draw quad using vertex data and index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ss->index_buffers[ascii]);
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);

        // get clip
        LRect* clip = (LRect*)vector_get(ss->clips, ascii);
        // move over
        glTranslatef(clip->w, 0.f, 0.f);
        render_x += clip->w;
      }
    }

    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // diable vertex and texture coordinate arrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
  }
}
