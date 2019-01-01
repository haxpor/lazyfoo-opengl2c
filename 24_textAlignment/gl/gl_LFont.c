#include "gl_LFont.h"
#include <stdlib.h>
#include <stddef.h>
#include "SDL_log.h"
#include "foundation/vector.h"
#include "gl/gl_LTexture_internals.h"
#include FT_BITMAP_H
#include "gl/gl_LFont_internals.h"

// spacing when render between character in pixel
#define BETWEEN_CHAR_SPACING 4

// freetype font library
// single shared variable for all instance of gl_LFont during lifetime of application
static FT_Library freetype_library_ = NULL;

static void init_defaults_(gl_LFont* font);
static void free_internals_(gl_LFont* font);
static void report_freetype_error_(const FT_Error* error);

void init_defaults_(gl_LFont* font)
{
  font->spritesheet = NULL;
  font->space = 0.f;
  font->line_height = 0.f;
  font->newline = 0.f;
}

void report_freetype_error_(const FT_Error* error)
{
  SDL_Log("FreeType error with code: %X", *error);
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

  // now gl_LTexture supports 8-bit grayscale image
  // so black color is 0x0
  const GLubyte black_pixel = 0x00;

  // get rid of the font if it exists
  gl_LFont_free_font(font);

  // image pixels loaded
  gl_LTexture* texture = font->spritesheet->ltexture;
  // load from grayscale 8-bit image
  if (!gl_LTexture_load_pixels_from_file8(texture, path))
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
          if (gl_LTexture_get_pixel8(texture, p_x, p_y) != black_pixel)
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
          if (gl_LTexture_get_pixel8(texture, p_x, p_y) != black_pixel)
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
          if (gl_LTexture_get_pixel8(texture, p_x, p_y) != black_pixel)
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
          if (gl_LTexture_get_pixel8(texture, p_x, p_y) != black_pixel)
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

  // create texture from manipulated pixels
  if (!gl_LTexture_load_texture_from_precreated_pixels8(texture))
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

bool gl_LFont_load_freetype(gl_LFont* font, const char* path, GLuint pixel_size)
{
  // free previously loaded font
  gl_LFont_free_font(font);

  // init freetype
  FT_Error error = 0;

  error = FT_Init_FreeType(&freetype_library_);
  if (error)
  {
    report_freetype_error_(&error);
    return false;
  }

  // get cell dimensions
  GLuint cell_width = 0;
  GLuint cell_height = 0;
  int max_bearing = 0;
  int min_hang = 0;

  // character data
  // this is an array of pointer to gl_LTexture
  gl_LTexture* bitmaps[256];
  FT_Glyph_Metrics metrics[256];
  
  // load face
  FT_Face face = NULL;
  error = FT_New_Face(freetype_library_, path, 0, &face);
  // error 0 means success for FreeType
  if (error)
  {
    report_freetype_error_(&error);
    return false;
  }

  // set face size
  error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
  if (error)
  {
    report_freetype_error_(&error);
    return false;
  }

  // go through extended ASCII to get glyph data
  for (int i=0; i<256; i++)
  {
    // load and render glyph
    error = FT_Load_Char(face, i, FT_LOAD_RENDER);
    if (error)
    {
      report_freetype_error_(&error);
      // report error but still keep going until finish all glyphs
      continue;
    }

    // get metrics
    metrics[i] = face->glyph->metrics;

    // initialize gl_LTexture inside bitmaps array
    bitmaps[i] = gl_LTexture_new();

    // copy glyph bitmap
    gl_LTexture_copy_pixels8(bitmaps[i], face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows);

    // calculate max bearing
    // as in http://lazyfoo.net/tutorials/OpenGL/23_freetype_fonts/index.php
    // author claims that 1 point = 64 pixels
    if (metrics[i].horiBearingY / 64 > max_bearing)
    {
      max_bearing = metrics[i].horiBearingY / 64;
    }

    // calculate max width
    if (metrics[i].width / 64 > cell_width)
    {
      cell_width = metrics[i].width / 64;
    }

    // calculate glyph hang
    int glyph_hang = (metrics[i].horiBearingY - metrics[i].height) / 64;
    if (glyph_hang < min_hang)
    {
      min_hang = glyph_hang;
    }
  }

  // create bitmap font
  cell_height = max_bearing - min_hang;
  // 16 by 16 cells in creation
  gl_LTexture_create_pixels8(font->spritesheet->ltexture, cell_width * 16, cell_height * 16);
  
  // begin creating bitmap font
  GLuint current_char = 0;
  LRect next_clip = { 0.f, 0.f, cell_width, cell_height };

  // blitting coordinates
  int b_x = 0;
  int b_y = 0;

  // go through cell rows
  for (unsigned int rows = 0; rows < 16; rows++)
  {
    // go through each cell column in the row
    for (unsigned int cols = 0; cols < 16; cols++)
    {
      // set base offsets
      b_x = cell_width * cols;
      b_y = cell_height * rows;

      // initialize clip
      next_clip.x = b_x;
      next_clip.y = b_y;
      next_clip.w = metrics[current_char].width / 64;
      next_clip.h = cell_height;

      // blit character
      gl_LTexture_blit_pixels8(bitmaps[current_char], b_x, b_y + max_bearing - metrics[current_char].horiBearingY / 64, font->spritesheet->ltexture);

      // go to the next character
      vector_add(font->spritesheet->clips, &next_clip);
      current_char++;
    }
  }

  // we are done with bitmaps
  // free them all now
  // free all allocated bitmaps
  for (int i=0; i<256; i++)
  {
    if (bitmaps[i] != NULL)
    {
      gl_LTexture_free(bitmaps[i]);
      bitmaps[i] = NULL;
    }
  }

  // make texture power of two
  gl_LTexture_pad_pixels8(font->spritesheet->ltexture);

  // create texture
  if (!gl_LTexture_load_texture_from_precreated_pixels8(font->spritesheet->ltexture))
  {
    SDL_Log("Unable to create texture from pre-created pixels8");
    return false;
  }

  // build vertex buffer from sprite sheet data
  if (!gl_LSpritesheet_generate_databuffer(font->spritesheet))
  {
    SDL_Log("Unable to geneate databuffer");
    return false;
  }

  // set texture wrap
  glBindTexture(GL_TEXTURE_2D, font->spritesheet->ltexture->texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glBindTexture(GL_TEXTURE_2D, 0);

  // set spacing variables
  font->space = cell_width / 2.0f;
  font->line_height = cell_height;
  font->newline = max_bearing;

  // free face
  FT_Done_Face(face);
  face = NULL;

  // free freetype
  FT_Done_FreeType(freetype_library_);
  freetype_library_ = NULL;

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
        glTranslatef(clip->w + BETWEEN_CHAR_SPACING, 0.f, 0.f);
        render_x += clip->w + BETWEEN_CHAR_SPACING;
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

void gl_LFont_render_textex(gl_LFont* font, const char* text, GLfloat x, GLfloat y, const LSize* area_size, int align)
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

  // if the text needs to be aligned
  if (area_size != NULL)
  {
    // correct empty alignment
    if (align == 0)
    {
      align = gl_LFont_TEXT_ALIGN_LEFT | gl_LFont_TEXT_ALIGN_TOP;
    }

    // handle horizontal alignment
    if (align & gl_LFont_TEXT_ALIGN_CENTERED_H)
    {
      render_x = x + (area_size->w - gl_LFont_string_width(font, text)) / 2.f;
    }
    else if (align & gl_LFont_TEXT_ALIGN_RIGHT)
    {
      render_x = x + area_size->w - gl_LFont_string_width(font, text);
    }

    // handle vertical alignment
    if (align & gl_LFont_TEXT_ALIGN_CENTERED_V)
    {
      render_y = y + (area_size->h - gl_LFont_string_height(font, text)) / 2.f;
    }
    else if (align & gl_LFont_TEXT_ALIGN_BOTTOM)
    {
      render_y = y + area_size->h - gl_LFont_string_height(font, text);
    }
  }

  glTranslatef(render_x, render_y, 0.f);

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
    // newlines
    else if (text[i] == '\n')
    {
      // handle horizontal alignment
      GLfloat target_x = x;

      if (area_size != NULL)
      {
        // handle horizontal alignment
        if (align & gl_LFont_TEXT_ALIGN_CENTERED_H)
        {
          target_x += (area_size->w - gl_LFont_string_width(font, text + i + 1)) / 2.f;
        }
        else if (align & gl_LFont_TEXT_ALIGN_RIGHT)
        {
          target_x += area_size->w - gl_LFont_string_width(font, text + i + 1);
        }
      }
      glTranslatef(target_x - render_x, font->newline, 0.f);
      render_y += font->newline;
      render_x += target_x - render_x;
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
      glTranslatef(clip->w + BETWEEN_CHAR_SPACING, 0.f, 0.f);
      render_x += clip->w + BETWEEN_CHAR_SPACING;
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

GLfloat gl_LFont_string_width(gl_LFont* font, const char* string)
{
  GLfloat width = 0.f;

  // go through string
  for (int i=0; string[i] != '\0' && string[i] != '\n'; i++)
  {
    // space
    if (string[i] == ' ')
    {
      width += font->space + BETWEEN_CHAR_SPACING;
    }
    // character
    else
    {
      // get ASCII
      GLuint ascii = (unsigned char)string[i];
      // note: will possibly be bottleneck later as it needs to convert data type here
      // consider has a specific type of vector here later?
      width += (*(LRect*)vector_get(font->spritesheet->clips, ascii)).w + BETWEEN_CHAR_SPACING;
    }
  } 

  return width;
}

GLfloat gl_LFont_string_height(gl_LFont* font, const char* string)
{
  GLfloat height = font->line_height;

  // go through string
  for (int i=0; string[i] != '\0'; i++)
  {
    // more space accumulated
    if (string[i] == '\n')
    {
      height += font->line_height;
    }
  }

  return height;
}
