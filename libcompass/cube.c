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
  { 7, 4, 6, 5 },
  { 6, 5, 2, 1 },
  { 2, 1, 3, 0 },
  { 3, 0, 7, 4 },
  { 3, 7, 2, 6 },
  { 4, 0, 5, 1 }
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
  { 1.0, 0.0 },
  { 0.0, 0.0 },
  { 1.0, 1.0 },
  { 0.0, 1.0 }
};

/* Create a cube.  */

object *
cube_create (float size)
{
  unsigned int face, vert;
  strip *cube_strips = NULL;
  
  for (face = 0; face < 6; face++)
    {
      cube_strips = strip_cons (cube_strips, 4, ALLOC_GEOMETRY | ALLOC_NORMALS
				| ALLOC_TEXCOORDS);
      
      for (vert = 0; vert < 4; vert++)
        {
          vec_scale ((*cube_strips->start)[vert],
		     points[tristrips[face][vert]], size);
	  memcpy ((*cube_strips->normals)[vert], face_normals[face],
		  sizeof (float) * 3);
	  memcpy ((*cube_strips->texcoords)[vert], texcoords[vert],
		  sizeof (float) * 2);
	}
    }
  
  return object_create_default (cube_strips);
}
