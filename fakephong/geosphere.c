#include <alloca.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <kos.h>

#include "vector.h"
#include "restrip.h"

/***
	 1           3           5
	  ________________________
	 /\    /\    /\    ,`    /
  B     /  \  /  \  /  \  /  `  /
 ___   /____\/____\/____\/____`/
      /\    /\    /\    /\    /
  A  /  \  /  \  /  \  /  \  /
    /____\/____\/____\/____\/
    0           2           4
 
***/

static void
midpoint (float *out, float *a, float *b)
{
  out[0] = (a[0] + b[0]) / 2.0;
  out[1] = (a[1] + b[1]) / 2.0;
  out[2] = (a[2] + b[2]) / 2.0;
  vec_normalize (&out[0], &out[0]);
}

static float *
subdivide (float in_points[][3], int strips_in, int strip_length, int recur,
	   int *strips_out_p, int *strip_length_out_p)
{
  int strips_out = strips_in * 2;
  int quads_in = strip_length - 2;
  int strip_len_out = (quads_in * 2) + 2;
  int points_out = strips_out * strip_len_out;
  int a_idx = 0, b_idx = points_out / 2;
  float out_points[points_out][3];
  int i, j;
  float mid_last[3];
  
  for (j = 0; j < strips_in; j++)
    {
      int stripbase = j * strip_length;
      /*int c;*/

#if 0
      dbgio_printf ("recur %d, strip %d: input points\n", recur, j);
      for (c = 0; c < strip_length; c++)
        dbgio_printf ("[ %f %f %f ]\n", (double) in_points[stripbase + c][0],
					(double) in_points[stripbase + c][1],
					(double) in_points[stripbase + c][2]);
      dbgio_printf ("\n");
#endif

      for (i = 0; i < strip_length - 2; i++)
	{
	  float mid_01[3], mid_02[3];

	  midpoint (mid_01, in_points[stripbase + i],
		    in_points[stripbase + i + 1]);
	  midpoint (mid_02, in_points[stripbase + i],
		    in_points[stripbase + i + 2]);

	  if ((i & 1) == 0)
            {
	      /* A part.  */
	      memcpy (out_points[a_idx++], in_points[stripbase + i],
		      3 * sizeof (float));
	      memcpy (out_points[a_idx++], mid_01, 3 * sizeof (float));
	      memcpy (out_points[a_idx++], mid_02, 3 * sizeof (float));

	      /* B part.  */
	      memcpy (out_points[b_idx++], mid_01, 3 * sizeof (float));
	      memcpy (out_points[b_idx++], in_points[stripbase + i + 1],
		      3 * sizeof (float));
	    }
	  else
	    {
	      /* A part.  */
	      memcpy (out_points[a_idx++], mid_01, 3 * sizeof (float));

	      /* B part.  */
	      memcpy (out_points[b_idx++], mid_01, 3 * sizeof (float));
	      memcpy (out_points[b_idx++], mid_02, 3 * sizeof (float));
	    }
	}

      midpoint (mid_last, in_points[stripbase + strip_length - 2],
		in_points[stripbase + strip_length - 1]);

      /* Finish off A part.  */
      memcpy (out_points[a_idx++], in_points[stripbase + strip_length - 2],
	      3 * sizeof (float));
      memcpy (out_points[a_idx++], mid_last, 3 * sizeof (float));

      /* Finish off B part.  */
      memcpy (out_points[b_idx++], mid_last, 3 * sizeof (float));
      memcpy (out_points[b_idx++], in_points[stripbase + strip_length - 1],
	      3 * sizeof (float));
    }
  
  if (recur == 0)
    {
      float *out_pts_copy = malloc (points_out * sizeof (float) * 3);

      *strips_out_p = strips_out;
      *strip_length_out_p = strip_len_out;

      memcpy (out_pts_copy, out_points, points_out * sizeof (float) * 3);

      return out_pts_copy;
    }
  else
    return subdivide (out_points, strips_out, strip_len_out, recur - 1,
		      strips_out_p, strip_length_out_p);
}

float *
make_geosphere (int depth, int *strips_p, int *strip_length_p)
{
  enum
    {
      T_TOP,
      T_LEFT,
      T_FRONT,
      T_RIGHT,
      T_BACK,
      T_BOTTOM
    };
  float points[][3] =
    {
      {0.0, 1.0, 0.0},	/* top */
      {-1.0, 0.0, 0.0},	/* left */
      {0.0, 0.0, -1.0},	/* front */
      {1.0, 0.0, 0.0},	/* right */
      {0.0, 0.0, 1.0},	/* back */
      {0.0, -1.0, 0.0}	/* bottom */
    };
  float in_points[16][3];
  int strips[][4] =
    {
      { T_BOTTOM, T_LEFT, T_FRONT, T_TOP },
      { T_BOTTOM, T_FRONT, T_RIGHT, T_TOP },
      { T_BOTTOM, T_RIGHT, T_BACK, T_TOP },
      { T_BOTTOM, T_BACK, T_LEFT, T_TOP }
    };
  int i, j;
  
  for (j = 0; j < 4; j++)
    for (i = 0; i < 4; i++)
      memcpy (&in_points[j * 4 + i][0], &points[strips[j][i]][0],
	      3 * sizeof (float));

  return subdivide (in_points, 4, 4, depth, strips_p, strip_length_p);
}

strip *
strips_for_geosphere (float *data, int num_strips, int strip_length)
{
  strip *begin = NULL, *end = NULL;
  unsigned int i;
  
  for (i = 0; i < num_strips; i++)
    {
      strip *newstrip = malloc (sizeof (strip));
      
      if (begin == NULL)
	begin = newstrip;

      if (end == NULL)
        end = newstrip;
      else
        end->next = newstrip;

      newstrip->start = (float (*)[][3]) &data[i * strip_length * 3];
      newstrip->length = strip_length;
      newstrip->next = NULL;
      
      end = newstrip;
    }
  
  return begin;
}
