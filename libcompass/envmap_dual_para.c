#include "object.h"
#include "envmap_dual_para.h"

/* X_VERTEX is a vertex transformed by the modelview matrix.  X_NORMAL is a
   vertex transformed by the normal matrix (transpose-inverse of rotation part
   of modelview).  C_EYEPOS is the eye position, transformed by the camera
   matrix.  INVCAMERA is the inverse (i.e. transpose, if orthogonal) of the
   rotation part of the camera matrix.  */

void
envmap_dual_para_texcoords (float *texc_f, float *texc_b,
			    float x_vertex[3], float x_normal[3],
			    float c_eyepos[3], matrix_t invcamera)
{
  float eye_to_vertex[3], tmp[3], reflection[4], unrot[4], eye_norm_dot;
  float x, y, z, x_f, y_f, z_f, x_b, y_b, z_b;
  
  vec_sub (eye_to_vertex, &c_eyepos[0], &x_vertex[0]);
  vec_normalize (eye_to_vertex, eye_to_vertex);
  
  eye_norm_dot = vec_dot (eye_to_vertex, &x_normal[0]);
  vec_scale (tmp, x_normal, 2.0 * eye_norm_dot);
  
  vec_sub (reflection, tmp, eye_to_vertex);
  reflection[3] = 1.0;
  vec_transform (unrot, (float *) &invcamera[0][0], reflection);
  x = unrot[0];
  y = unrot[1];
  z = unrot[2];
  
  x_f = x;
  y_f = -y;
  z_f = -1.0 - z;
  
  x_f = 0.5 + 0.5 * (x_f / z_f);
  y_f = 0.5 + 0.5 * (y_f / z_f);
  
  texc_f[0] = x_f;
  texc_f[1] = y_f;
  texc_f[2] = z;
  
  x_b = x;
  y_b = y;
  z_b = 1.0 - z;
  
  x_b = 0.5 + 0.5 * (x_b / z_b);
  y_b = 0.5 + 0.5 * (y_b / z_b);
  
  texc_b[0] = x_b;
  texc_b[1] = y_b;
  texc_b[2] = z;
}

/* A triangle can be rendered wholly with the front texture, or wholly with
   the back texture, or with a mixture of both.  Use the following buckets for
   these:
   
     0: front
     1: back
     2: both front & back
   
   Notice this allows single-pass rendering for 0/1.  */

int
envmap_classify_triangle (float tri[3][3], int clockwise, vertex_attrs *attrs)
{
  if (attrs[0].env_map.texc_f[2] > 0
      && attrs[1].env_map.texc_f[2] > 0
      && attrs[2].env_map.texc_f[2] > 0)
    return 0;
  else if (attrs[0].env_map.texc_b[2] < 0
           && attrs[1].env_map.texc_b[2] < 0
	   && attrs[2].env_map.texc_b[2] < 0)
    return 1;
  else
    return 2;
}
