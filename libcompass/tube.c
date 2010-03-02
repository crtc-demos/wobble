#include "tube.h"

object *
allocate_tube (int rows, int segments)
{
  int r;
  strip *prev_strip = NULL;
  
  for (r = 0; r < rows; r++)
    {
      strip *newstrip = strip_cons (prev_strip, segments * 2 + 2,
				    ALLOC_GEOMETRY | ALLOC_NORMALS);
      prev_strip = newstrip;
    }
  
  return object_create_default (prev_strip);
}

void
fill_tube_data (object *obj, int rows, int segments, float rot1)
{
  int r, s;
  strip *strlist = obj->striplist;
  
  for (r = 0; r < rows; r++)
    {
      float (*str)[][3];
      float (*norm)[][3];
      float ang0 = rot1 + 4 * (float) r / (float) rows;
      float ang1 = rot1 + 4 * (float) (r + 1) / (float) rows;
      float mag = 1.0 + 0.35 * fsin (ang0);
      float mag1 = 1.0 + 0.35 * fsin (ang1);

      assert (strlist);
      
      str = strlist->start;
      norm = strlist->normals;
      strlist->inverse = 1;
      
      for (s = 0; s <= segments; s++)
        {
	  float ang = 2 * M_PI * (float) s / (float) segments;
	  float cosang = fcos (ang);
	  float sinang = fsin (ang);
	  float norma[3];
	  
	  (*str)[s * 2][0] = mag * cosang;
	  (*str)[s * 2][1] = (2.0 * (float) r / (float) rows) - 1.0;
	  (*str)[s * 2][2] = mag * sinang;

	  (*str)[s * 2 + 1][0] = mag1 * cosang;
	  (*str)[s * 2 + 1][1] = (2.0 * (float) (r + 1) / (float) rows) - 1.0;
	  (*str)[s * 2 + 1][2] = mag1 * sinang;
	  
	  norma[0] = cosang;
	  norma[1] = 0.35 * fcos (ang0);
	  norma[2] = sinang;
	  
	  vec_normalize (&(*norm)[s * 2][0], &norma[0]);
	  
	  norma[1] = 0.35 * fcos (ang1);

	  vec_normalize (&(*norm)[s * 2 + 1][0], &norma[0]);
	}
      
      strlist = strlist->next;
    }
}
