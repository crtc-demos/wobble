#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include <kos.h>

#include "vector.h"
#include "object.h"

object *
torus_create (float outer, float inner, int minor_steps, int major_steps)
{
  int major;
  float major_ang = 0.0, major_incr = 2.0 * M_PI / major_steps;
  float first_row[minor_steps + 1][2][3], prev_row[minor_steps + 1][2][3];
  strip *begin = NULL, *end = NULL;
  
  for (major = 0; major <= major_steps; major++, major_ang += major_incr)
    {
      int minor;
      float fsin_major = fsin (major_ang);
      float fcos_major = fcos (major_ang);
      float maj_x = fcos_major * outer;
      float maj_y = fsin_major * outer;
      float minor_ang = 0.0, minor_incr = 2.0 * M_PI / minor_steps;
      float first[2][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };
      float last[2][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };
      unsigned int num_vertices = minor_steps * 4, vout = 0;
      float (*vert)[][3] = NULL;
      float (*norm)[][3] = NULL;
      strip *str;

      if (major != 0)
        {
	  vert = malloc (sizeof (float) * 3 * num_vertices);
	  norm = malloc (sizeof (float) * 3 * num_vertices);
	}

      for (minor = 0; minor <= minor_steps; minor++, minor_ang += minor_incr)
        {
	  float fsin_minor = fsin (minor_ang);
	  float fcos_minor = fcos (minor_ang);
	  float min[2][3], orig_min_x;
	  
	  min[1][0] = fcos_major * fsin_minor;
	  min[1][1] = fsin_major * fsin_minor;
	  min[1][2] = fcos_minor;
	  
	  /* 2D rotation is just:
	  
	     [ cos T  -sin T ] (x)
	     [ sin T   cos T ] (y).
	     
	     y is zero, so the corresponding terms disappear.  */

	  orig_min_x = fsin_minor * inner;
	  min[0][0] = fcos_major * orig_min_x + maj_x;
	  min[0][1] = fsin_major * orig_min_x + maj_y;
	  min[0][2] = fcos_minor * inner;

	  if (major == 0)
	    {
	      memcpy (&prev_row[minor][0][0], &min[0][0], 6 * sizeof (float));
	      memcpy (&first_row[minor][0][0], &min[0][0], 6 * sizeof (float));
	      continue;
	    }
	  
	  if (minor == 0)
	    memcpy (&first[0], &min[0], 6 * sizeof (float));
	  else
	    {
	      int minor_wrapped = (minor == minor_steps) ? 0 : minor;

	      /* Polygon is formed from:
	           prev_row[minor-1]  ,-> last_xyz
		        v            /       v
		   prev_row[minor] -'	  min_xyz.  */
	      
	      memcpy (&(*vert)[vout], &prev_row[minor - 1][0],
		      sizeof (float) * 3);
	      memcpy (&(*norm)[vout++], &prev_row[minor - 1][1],
		      sizeof (float) * 3);
	      
	      memcpy (&(*vert)[vout], &prev_row[minor][0], sizeof (float) * 3);
	      memcpy (&(*norm)[vout++], &prev_row[minor][1],
		      sizeof (float) * 3);
	      
	      if (major == major_steps)
	        {
		  memcpy (&(*vert)[vout], &first_row[minor - 1][0],
			  sizeof (float) * 3);
		  memcpy (&(*norm)[vout++], &first_row[minor - 1][1],
			  sizeof (float) * 3);
		  
		  memcpy (&(*vert)[vout], &first_row[minor_wrapped][0],
			  sizeof (float) * 3);
		  memcpy (&(*norm)[vout++], &first_row[minor_wrapped][1],
			  sizeof (float) * 3);
		}
	      else
	        {
		  memcpy (&(*vert)[vout], last[0], sizeof (float) * 3);
		  memcpy (&(*norm)[vout++], last[1], sizeof (float) * 3);

		  //glColor4ub (0, 255, 0, 0);
		  if (minor == minor_steps)
		    {
		      memcpy (&(*vert)[vout], first[0], sizeof (float) * 3);
		      memcpy (&(*norm)[vout++], first[1], sizeof (float) * 3);
		    }
		  else
		    {
		      memcpy (&(*vert)[vout], min[0], sizeof (float) * 3);
		      memcpy (&(*norm)[vout++], min[1], sizeof (float) * 3);
		    }
		}

	      memcpy (&prev_row[minor - 1][0][0], &last[0][0],
		      6 * sizeof (float));
	    }

	  memcpy (&last[0][0], &min[0][0], 6 * sizeof (float));
	}

      memcpy (&prev_row[minor_steps][0][0], &prev_row[0][0],
	      6 * sizeof (float));

      if (major == 0)
        continue;

      if (vout != num_vertices)
        {
	  printf ("vout = %d, num_vertices = %d\n", vout, num_vertices);
	  assert (vout == num_vertices);
	}

      str = malloc (sizeof (strip));
      
      if (begin == NULL)
        begin = str;

      if (end == NULL)
        end = str;
      else
        end->next = str;

      str->start = vert;
      str->normals = norm;
      str->texcoords = NULL;
      str->length = vout;
      str->inverse = 0;
      str->v_attrs = NULL;
      str->s_attrs = NULL;
      str->next = NULL;

      end = str;
    }
  
  return object_create_default (begin);
}
