#include <kos.h>

#include "vector.h"
#include "object.h"

/***

     3______ 2
     /:    /|
   7/_____/6|
    |0:.. |.|1
    | ,   | /
   4|_____|/5

***/

static GLfloat points[][3] =
{
  {-1.0, -1.0,  1.0},
  { 1.0, -1.0,  1.0},
  { 1.0,  1.0,  1.0},
  {-1.0,  1.0,  1.0},
  {-1.0, -1.0, -1.0},
  { 1.0, -1.0, -1.0},
  { 1.0,  1.0, -1.0},
  {-1.0,  1.0, -1.0},
};

static int tristrips[][4] =
{
  { 4, 7, 5, 6 },
  { 5, 6, 1, 2 },
  { 1, 2, 0, 3 },
  { 0, 3, 4, 7 },
  { 7, 3, 6, 2 },
  { 0, 4, 1, 5 }
};

static GLfloat face_normals[][3] =
{
  { 0.0,  0.0, -1.0 },
  { 1.0,  0.0,  0.0 },
  { 0.0,  0.0,  1.0 },
  {-1.0,  0.0,  0.0 },
  { 0.0,  1.0,  0.0 },
  { 0.0, -1.0,  0.0 }
};

/* Texture coordinates are "upside down", so V goes from 0 (top)
   to 1 (bottom).  */
static GLfloat texcoords[][2] =
{
  { 0.0, 0.0 },
  { 1.0, 0.0 },
  { 0.0, 1.0 },
  { 1.0, 1.0 }
};

/* Create a cube.  Note: vertex data and normals are heap-allocated, but
   texture coords are static (might want a way of recording that in the object
   later).  */

object *
cube_create (float size)
{
  int face;
  strip *begin = NULL, *end = NULL;
  
  for (face = 0; face < 6; face++)
    {
      strip *str = calloc (1, sizeof (strip));
      int vert;
      
      str->length = 4;
      str->start = malloc (sizeof (float) * str->length * 3);
      
      for (vert = 0; vert < str->length; vert++)
        vec_scale ((*str->start)[vert], points[tristrips[face][vert]], size);
      
      str->normals = malloc (sizeof (float) * str->length * 3);
      
      for (vert = 0; vert < str->length; vert++)
        memcpy ((*str->normals)[vert], face_normals[face], sizeof (float) * 3);

      str->texcoords = texcoords;
      str->inverse = 0;
      str->v_attrs = NULL;
      str->s_attrs = calloc (1, sizeof (strip_attrs));
      str->next = NULL;

      if (begin == NULL)
        begin = str;
      
      if (end)
        end->next = str;

      end = str;
    }
  
  return object_create_default (begin);
}
