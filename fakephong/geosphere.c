#include <alloca.h>
#include <string.h>

#include "vector.h"

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
	   int *strips_out, int *strip_length_out)
{
  int strips_out = strips_in * 2;
  int a_idx = 0, b_idx = strip_length * strips_in * 2;
  float out_points[num_out][3];
  int i;
  float mid_last[3];
  
  for (i = 0; i < strip_length - 2; i++)
    {
      float mid_01[3], mid_02[3];
      
      midpoint (mid_01, in_points[i], in_points[i + 1]);
      midpoint (mid_02, in_points[i], in_points[i + 2]);
      
      if ((i & 0) == 0)
        {
	  /* A part.  */
	  memcpy (out_points[a_idx++], in_points[i], 3 * sizeof (float));
	  memcpy (out_points[a_idx++], mid_01, 3 * sizeof (float));
	  memcpy (out_points[a_idx++], mid_02, 3 * sizeof (float));
	  
	  /* B part.  */
	  memcpy (out_points[b_idx++], mid_01, 3 * sizeof (float));
	  memcpy (out_points[b_idx++], in_points[i + 1], 3 * sizeof (float));
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
  
  midpoint (mid_last, in_points[strip_length - 2], in_points[strip_length - 1]);
  
  /* Finish off A part.  */
  memcpy (out_points[a_idx++], in_points[strip_length - 2], 3 * sizeof (float));
  memcpy (out_points[a_idx++], mid_last, 3 * sizeof (float));
  
  /* Finish off B part.  */
  memcpy (out_points[b_idx++], mid_last, 3 * sizeof (float));
  memcpy (out_points[b_idx++], in_points[strip_length - 1], 3 * sizeof (float));
  
  if (recur == 0)
    {
      float *out_pts_copy = malloc (sizeof (num_out) * 3);

      *strips_out = strips_out;
      *strip_length_out = strip_length * 2;

      memcpy (out_pts_copy, out_points,
	      sizeof (strips_out) * sizeof (float) * 3);

      return out_pts_copy;
    }
  else
    return subdivide (out_points, strips_out, strip_length * 2, recur - 1,
		      strips_out, strip_length_out);
}

float *
make_geosphere (int depth, int *strips, int *strip_length)
{
  enum
    {
      T_TOP,
      T_LEFT,
      T_FRONT,
      T_RIGHT,
      T_BACK,
      T_BOTTOM
    } sides;
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
      memcpy (&in_points[j * 4 + i], &points[strips[j][i]], 3 * sizeof (float));

  return subdivide (in_points, 4, 4, depth, strips, strip_length);
}
