#include "vector.h"
#include "common_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vector* vector_new(int estimatedLen, int stride)
{
  assert(estimatedLen > 0);
  assert(stride > 0);

  // create vector
  vector* out = malloc(sizeof(vector));
  out->len = 0;
  out->mlen = estimatedLen;
  out->stride = stride;
  out->free_element = NULL; // default with no custom freeing function for its element
  
  // create buffer held by vector
  void* buf = malloc(stride * estimatedLen);
  // set to vector
  out->buffer = buf;

  return out;
}

void vector_add(vector* v, void* d)
{
  assert(v != NULL);
  assert(d != NULL);

  // if there is still space left to fill
  if (v->len < v->mlen) {
    // do memory copy from source to our buffer
    memcpy(v->buffer + v->len*v->stride, d, v->stride);
    v->len++;
  }
  // otherwise it need to allocate more space
  // only allocate more by 1 element
  else {
    v->buffer = realloc(v->buffer, (v->mlen + 1)*v->stride); 
    v->mlen++;
    memcpy(v->buffer + v->len*v->stride, d, v->stride);
    v->len++;
  }
}

void vector_remove(vector* v, int i)
{
  assert(v != NULL);
  assert(i < v->len);

  // do custom memory freeing if user supply such function
  if (v->free_element != NULL)
  {
    v->free_element(v->buffer + i*v->stride);
  }
  
  // shift element as one has been removed
  for (int index=i; index<v->len-1; index++) {
    memcpy(v->buffer + index*v->stride, v->buffer + (index+1)*v->stride, v->stride);
  }

  // write 0 for stride size for last element
  memset(v->buffer + (v->len-1)*v->stride, 0, v->stride);

  // update len
  // note: don't decrement mlen, we will only grow not shrink.
  // only shrink when free via vector_free()
  v->len--;
}

void* vector_get(vector* v, int i)
{
  assert(v != NULL);
  assert(i < v->len);

  return v->buffer + i*v->stride;
}

void vector_clear(vector* v)
{
  // only make sense if we do this operation when there's at least 1 element inside
  if (v->len > 0)
  {
    // if need to do custom memory freeing for element, then do it
    if (v->free_element != NULL)
    {
      // do it for all element
      for (int i=0; i<v->len; i++)
      {
        v->free_element(v->buffer + i*v->stride);
      }
    }

    // write 0 for all allocated memory (for safety)
    memset(v->buffer, 0, v->len*v->stride);

    // now all elements are not making sense if we still access it
    v->len = 0;
    // shirnk data down, set back to estimated number of element of 1
    v->mlen = 1;
    v->buffer = realloc(v->buffer, v->mlen*v->stride);
  }
}

void vector_free(vector* v)
{
  assert(v != NULL);

  // do custom memory freeing for all elements
  // if user supplies with such function
  if (v->free_element != NULL)
  {
    for (int i=0; i<v->len; i++)
    {
      v->free_element(v->buffer + i*v->stride);
    }
  }

  // free its buffer first
  if (v->buffer != NULL) {
    free(v->buffer);
    v->buffer = NULL;
  }

  // free the source
  free(v);
  v = NULL;
}
