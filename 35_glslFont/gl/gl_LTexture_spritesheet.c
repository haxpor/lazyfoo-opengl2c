#include "gl_LTexture_spritesheet.h"
#include "gl_ltextured_polygon_program2d.h"
#include "gl/gl_util.h"
#include <stdlib.h>
#include <stdlib.h>
#include <stddef.h>
#include "SDL_log.h"

static void init_defaults(gl_LSpritesheet* spritesheet);
static void free_internals(gl_LSpritesheet* spritesheet);

void init_defaults(gl_LSpritesheet* spritesheet)
{
  spritesheet->ltexture = NULL;
  spritesheet->clips = NULL;
  spritesheet->vertex_data_buffer = 0;
  spritesheet->index_buffers = NULL;
}

void free_internals(gl_LSpritesheet* spritesheet)
{
  if (spritesheet->ltexture != NULL)
  {
    // use gl_LTexture's free function to help on freeing this
    gl_LTexture_free(spritesheet->ltexture);
    spritesheet->ltexture = NULL;
  }

  if (spritesheet->clips != NULL)
  {
    // use vector's free function to help on freeing this
    vector_free(spritesheet->clips);
    spritesheet->clips = NULL;
  }
}

gl_LSpritesheet* gl_LSpritesheet_new(gl_LTexture* ltexture)
{
  gl_LSpritesheet* out = malloc(sizeof(gl_LSpritesheet));
  init_defaults(out);

  // init
  out->ltexture = ltexture;
  out->clips = vector_new(1, sizeof(LRect));
  // no need to set free_element for vector this time as LRect is pure value type struct

  return out;
}

void gl_LSpritesheet_free(gl_LSpritesheet* spritesheet)
{
  // free sheet first
  gl_LSpritesheet_free_sheet(spritesheet);
  
  // free internals
  // the underlying managed gl_LTexture and vector will be freed inside this call
  free_internals(spritesheet);

  free(spritesheet);
  spritesheet = NULL;
}

int gl_LSpritesheet_add_clipsprite(gl_LSpritesheet* spritesheet, const LRect* new_clip)
{
  // add a new clip then return its index
  vector_add(spritesheet->clips, (void*)new_clip);
  return spritesheet->clips->len - 1;
}

LRect gl_LSpritesheet_get_clip(gl_LSpritesheet* spritesheet, int index)
{
  return *(LRect*)vector_get(spritesheet->clips, index);
}

bool gl_LSpritesheet_generate_databuffer(gl_LSpritesheet* spritesheet)
{
  // if there is a texture loaded, and clips to make vertex data from
  if (spritesheet->ltexture->texture_id != 0 && spritesheet->clips->len > 0)
  {
    // allocate vertex buffer data
    const int total_sprites = spritesheet->clips->len;

    // yep we can use variable length array declaration in C99
    LVertexData2D vertex_data[total_sprites * 4];
    // allocate buffer for index buffer
    spritesheet->index_buffers = malloc(total_sprites * 4 * sizeof(GLuint));

    // allocate vertex data buffer name
    glGenBuffers(1, &spritesheet->vertex_data_buffer);
    // allocate index buffer 
    glGenBuffers(total_sprites, spritesheet->index_buffers);

    // go through clips
    GLfloat texture_pwidth = spritesheet->ltexture->physical_width_;
    GLfloat texture_pheight = spritesheet->ltexture->physical_height_;
    GLuint sprite_indices[4] = {0, 0, 0, 0};

    for (int i=0; i<total_sprites; i++)
    {
      // initialize indices
      int base = i * 4;
      sprite_indices[0] = base;
      sprite_indices[1] = base + 1;
      sprite_indices[2] = base + 2;
      sprite_indices[3] = base + 3;

      // initialize vertex
      // get clip for current sprite
      LRect clip = *(LRect*)vector_get(spritesheet->clips, i);

      // calculate texture coordinate
      GLfloat tex_left = clip.x/texture_pwidth + 0.5/texture_pwidth;
      GLfloat tex_right = (clip.x+clip.w)/texture_pwidth - 0.5/texture_pwidth;
      GLfloat tex_top = clip.y/texture_pheight + 0.5/texture_pheight;
      GLfloat tex_bottom = (clip.y+clip.h)/texture_pheight - 0.5/texture_pheight;

      // top left
      vertex_data[sprite_indices[0]].position.x = 0.f;
      vertex_data[sprite_indices[0]].position.y = 0.f;

      vertex_data[sprite_indices[0]].texcoord.s = tex_left;
      vertex_data[sprite_indices[0]].texcoord.t = tex_top;

      // top right
      vertex_data[sprite_indices[1]].position.x = clip.w;
      vertex_data[sprite_indices[1]].position.y = 0.f;

      vertex_data[sprite_indices[1]].texcoord.s = tex_right;
      vertex_data[sprite_indices[1]].texcoord.t = tex_top;

      // bottom right
      vertex_data[sprite_indices[2]].position.x = clip.w;
      vertex_data[sprite_indices[2]].position.y = clip.h;

      vertex_data[sprite_indices[2]].texcoord.s = tex_right;
      vertex_data[sprite_indices[2]].texcoord.t = tex_bottom;

      // bottom left
      vertex_data[sprite_indices[3]].position.x = 0.f;
      vertex_data[sprite_indices[3]].position.y = clip.h;

      vertex_data[sprite_indices[3]].texcoord.s = tex_left;
      vertex_data[sprite_indices[3]].texcoord.t = tex_bottom;

      // bind sprite index buffer data
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spritesheet->index_buffers[i]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), sprite_indices, GL_STATIC_DRAW);

			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				SDL_Log("Error opengl: %s", gl_util_error_string(error));
				return false;
			}
    }

    // bind vertex data
    glBindBuffer(GL_ARRAY_BUFFER, spritesheet->vertex_data_buffer);
    glBufferData(GL_ARRAY_BUFFER, total_sprites * 4 * sizeof(LVertexData2D), vertex_data, GL_STATIC_DRAW);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			SDL_Log("Error opengl: %s", gl_util_error_string(error));
			return false;
		}
  }
  else
  {
    if (spritesheet->ltexture->texture_id == 0)
    {
      SDL_Log("No texture to render with!");
    }

    if (spritesheet->clips->len <= 0)
    {
      SDL_Log("No clips to generate vertex data from");
    }

    return false;
  }

  return true;
}

void gl_LSpritesheet_free_sheet(gl_LSpritesheet* spritesheet)
{
  // clear vertex buffer
  if (spritesheet->vertex_data_buffer != 0)
  {
    glDeleteBuffers(1, &spritesheet->vertex_data_buffer);
    spritesheet->vertex_data_buffer = 0;
  }

  // clear index buffer
  if (spritesheet->index_buffers != NULL)
  {
    glDeleteBuffers(spritesheet->clips->len, spritesheet->index_buffers);
    // since we dynamically allocate for index buffers, we free them here
    free(spritesheet->index_buffers);
    spritesheet->index_buffers = NULL;
  }

  // clear clips
  vector_clear(spritesheet->clips);
}

void gl_LSpritesheet_render_sprite(gl_LSpritesheet* spritesheet, int index, GLfloat x, GLfloat y)
{
  // save the current modelview matrix
  mat4 original_modelview_matrix;
  // copy original modelview matrix, then we work on top of modelview matrix of shared_textured_shaderprogram
  glm_mat4_copy(shared_textured_shaderprogram->modelview_matrix, original_modelview_matrix);
  
	// move to rendering position
  glm_translate(shared_textured_shaderprogram->modelview_matrix, (vec3){x, y, 0.f});
  // issue update to gpu
  gl_ltextured_polygon_program2d_update_modelview_matrix(shared_textured_shaderprogram);

  // set texture
  glBindTexture(GL_TEXTURE_2D, spritesheet->ltexture->texture_id);

  // enable all attribute pointers
  // use shared global variable of shader for gl_LTexture here
  gl_ltextured_polygon_program2d_enable_attrib_pointers(shared_textured_shaderprogram);

  // bind vertex data
  glBindBuffer(GL_ARRAY_BUFFER, spritesheet->vertex_data_buffer);

  // set texture coordinate attrib pointer
  gl_ltextured_polygon_program2d_set_vertex_pointer(shared_textured_shaderprogram, sizeof(LVertexData2D), (GLvoid*)offsetof(LVertexData2D, texcoord));
  // set vertex data attrib pointer
  gl_ltextured_polygon_program2d_set_texcoord_pointer(shared_textured_shaderprogram, sizeof(LVertexData2D), (GLvoid*)offsetof(LVertexData2D, position));

  // bind index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spritesheet->index_buffers[index]);
  // draw using data from vertex and index buffer
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

  // disable all attribute pointers
  gl_ltextured_polygon_program2d_disable_attrib_pointers(shared_textured_shaderprogram);

  // set modelview matrix back to original one
  glm_mat4_copy(original_modelview_matrix, shared_textured_shaderprogram->modelview_matrix);
}
