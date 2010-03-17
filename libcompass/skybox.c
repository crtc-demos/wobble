#include "skybox.h"

/***
          +
          |     +
	  5    /
       6______7
       /:  3 /|
     2/_____/3|
 -< 0 |4:_ _|_|51 >+
      |, 2  | /
     0|_____|/1
	  4
      /  
     -    |
          -
***/

object *
create_skybox (float radius, pvr_ptr_t textures[6], unsigned int xsize,
	       unsigned int ysize)
{
  strip *newstrip = NULL;
  object *newobj = NULL;
  
  const float vertices[8][3] = {
    { -radius, -radius, -radius },
    {  radius, -radius, -radius },
    { -radius,  radius, -radius },
    {  radius,  radius, -radius },
    { -radius, -radius,  radius },
    {  radius, -radius,  radius },
    { -radius,  radius,  radius },
    {  radius,  radius,  radius }
  };
  const float texcoords[4][2] = {
    { 1, 0 },
    { 0, 0 },
    { 1, 1 },
    { 0, 1 }
  };
  const int strips[6][4] = {
    { 2, 6, 0, 4 },
    { 7, 3, 5, 1 },
    { 3, 2, 1, 0 },
    { 6, 7, 4, 5 },
    { 5, 1, 4, 0 },
    { 6, 2, 7, 3 }
  };
  int str;
  
  for (str = 0; str < 6; str++)
    {
      int vtx;
      newstrip = strip_cons (newstrip, 4, ALLOC_GEOMETRY | ALLOC_TEXCOORDS);
      
      for (vtx = 0; vtx < 4; vtx++)
        {
	  memcpy (&(*newstrip->start)[vtx][0], &vertices[strips[str][vtx]][0],
		  3 * sizeof (float));
	  memcpy (&(*newstrip->texcoords)[vtx][0], &texcoords[vtx][0],
		  2 * sizeof (float));
	}

      newstrip->s_attrs = malloc (sizeof (strip_attrs));
      printf ("allocated s_attrs %p\n", newstrip->s_attrs);
      newstrip->s_attrs->texture = textures[str];
      newstrip->s_attrs->xsize = xsize;
      newstrip->s_attrs->ysize = ysize;
      /* Not very flexible!  */
      newstrip->s_attrs->txr_fmt = PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED;
      newstrip->inverse = 1;
    }

  newobj = object_create_default (newstrip);
  newobj->textured = 1;
  newobj->clip = 1;

  return newobj;
}
