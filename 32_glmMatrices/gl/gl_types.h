#ifndef gl_types_h_
#define gl_types_h_

#include "gl.h"

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat w;
  GLfloat h;
} LRect;

typedef struct
{
  GLfloat w;
  GLfloat h;
} LSize;

typedef struct
{
  GLfloat x;
  GLfloat y;
} LVector2D;

typedef struct
{
  GLfloat s;
  GLfloat t;
} LTexCoord2D;

typedef struct
{
  GLfloat x;
  GLfloat y;
} LVertexPos2D;

typedef struct
{
  LVector2D position;
  LTexCoord2D texcoord;
} LVertexData2D;

#endif
