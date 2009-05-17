#ifndef VECTOR_H
#define VECTOR_H 1

#ifndef FLOAT_TYPE
#define FLOAT_TYPE float
#endif

#include <assert.h>

#define DC_FAST_MATHS 1

#ifdef DC_FAST_MATHS
#include <dc/fmath.h>
#endif

#include <dc/matrix.h>
#include <GL/gl.h>

static __inline__ FLOAT_TYPE
vec_dot (FLOAT_TYPE *a, FLOAT_TYPE *b)
{
#ifdef DC_FAST_MATHS
  return fipr (a[0], a[1], a[2], 0.0, b[0], b[1], b[2], 0.0);
#else
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
#endif
}

static __inline__ void
vec_normalize (FLOAT_TYPE *dst, FLOAT_TYPE *in)
{
#ifdef DC_FAST_MATHS
  FLOAT_TYPE mag = fipr_magnitude_sqr (in[0], in[1], in[2], 0.0);
  FLOAT_TYPE rsqrt_mag = frsqrt (mag);
  dst[0] = in[0] * rsqrt_mag;
  dst[1] = in[1] * rsqrt_mag;
  dst[2] = in[2] * rsqrt_mag;
#else
  FLOAT_TYPE factor = sqrtf (in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
  
  if (factor != 0.0)
    factor = 1.0 / factor;
  
  dst[0] = in[0] * factor;
  dst[1] = in[1] * factor;
  dst[2] = in[2] * factor;
#endif
}

static __inline__ FLOAT_TYPE
vec_length (FLOAT_TYPE *vec)
{
#ifdef DC_FAST_MATHS
  return fsqrt (fipr_magnitude_sqr (vec[0], vec[1], vec[2], 0.0));
#else
  return sqrtf (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
#endif
}

static matrix_t b_mat __attribute__((aligned(32))) =
  {
    { 0.0, 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0, 0.0 }
  };

static __inline__ void
vec_cross (FLOAT_TYPE *dst, FLOAT_TYPE *a, FLOAT_TYPE *b)
{
#ifdef DC_FAST_MATHS
    
#if 0
  /* It's not clear that this wins: I've not measured performance.  */
  FLOAT_TYPE tmpx = fipr (a[1], a[2], 0.0, 0.0, b[2], -b[1], 0.0, 0.0);
  FLOAT_TYPE tmpy = fipr (a[2], a[0], 0.0, 0.0, b[0], -b[2], 0.0, 0.0);
  FLOAT_TYPE tmpz = fipr (a[0], a[1], 0.0, 0.0, b[1], -b[0], 0.0, 0.0);
  dst[0] = tmpx;
  dst[1] = tmpy;
  dst[2] = tmpz;
#else
  /*** Alternatively, we can do a ftrv like:
  
  [  0    b[2] -b[1] 0 ] [ a[0] ]
  [-b[2]   0    b[0] 0 ] [ a[1] ]
  [ b[1] -b[0]   0   0 ] [ a[2] ]
  [  0     0     0   0 ] [  0   ]
  
  ***/

  FLOAT_TYPE tmp1 = a[0], tmp2 = a[1], tmp3 = a[2], tmp4 = 0.0;
  
  /*b_mat[0][0] = b_mat[3][0] = b_mat[1][1] = b_mat[3][1] = 0.0;
  b_mat[2][2] = b_mat[3][2] = b_mat[0][3] = b_mat[1][3] = 0.0;
  b_mat[2][3] = b_mat[3][3] = 0.0;*/
  b_mat[1][0] = b[2];
  b_mat[2][0] = -b[1];
  b_mat[0][1] = -b[2];
  b_mat[2][1] = b[0];
  b_mat[0][2] = b[1];
  b_mat[1][2] = -b[0];
  mat_load (&b_mat);
  
  mat_trans_nodiv (tmp1, tmp2, tmp3, tmp4);
  dst[0] = tmp1;
  dst[1] = tmp2;
  dst[2] = tmp3;
  glKosMatrixDirty ();
#endif
#else
  FLOAT_TYPE dstc[3];
  
  dstc[0] = a[1] * b[2] - a[2] * b[1];
  dstc[1] = a[2] * b[0] - a[0] * b[2];
  dstc[2] = a[0] * b[1] - a[1] * b[0];
  dst[0] = dstc[0];
  dst[1] = dstc[1];
  dst[2] = dstc[2];
#endif
}

static __inline__ void
vec_scale (FLOAT_TYPE *dst, FLOAT_TYPE *src, FLOAT_TYPE scale)
{
  dst[0] = src[0] * scale;
  dst[1] = src[1] * scale;
  dst[2] = src[2] * scale;
}

static __inline__ void
vec_add (FLOAT_TYPE *dst, FLOAT_TYPE *a, FLOAT_TYPE *b)
{
  dst[0] = a[0] + b[0];
  dst[1] = a[1] + b[1];
  dst[2] = a[2] + b[2];
}

static __inline__ void
vec_sub (FLOAT_TYPE *dst, FLOAT_TYPE *a, FLOAT_TYPE *b)
{
  dst[0] = a[0] - b[0];
  dst[1] = a[1] - b[1];
  dst[2] = a[2] - b[2];
}

static __inline__ void
vec_transform (FLOAT_TYPE *dst, FLOAT_TYPE *mat, FLOAT_TYPE *src)
{
  assert (dst != src);

  dst[0] = mat[0] * src[0]
           + mat[4] * src[1]
	   + mat[8] * src[2] 
	   + mat[12] * src[3];
  dst[1] = mat[1] * src[0]
	   + mat[5] * src[1]
	   + mat[9] * src[2]
	   + mat[13] * src[3];
  dst[2] = mat[2] * src[0]
	   + mat[6] * src[1]
	   + mat[10] * src[2]
           + mat[14] * src[3];
  dst[3] = mat[3] * src[0]
	   + mat[7] * src[1]
	   + mat[11] * src[2]
           + mat[15] * src[3];
}

static __inline__ void
vec_transform_post (FLOAT_TYPE *dst, FLOAT_TYPE *src, FLOAT_TYPE *mat)
{
  assert (dst != src);

  dst[0] = mat[0] * src[0]
           + mat[1] * src[1]
	   + mat[2] * src[2] 
	   + mat[3] * src[3];
  dst[1] = mat[4] * src[0]
	   + mat[5] * src[1]
	   + mat[6] * src[2]
	   + mat[7] * src[3];
  dst[2] = mat[8] * src[0]
	   + mat[9] * src[1]
	   + mat[10] * src[2]
           + mat[11] * src[3];
  dst[3] = mat[12] * src[0]
	   + mat[13] * src[1]
	   + mat[14] * src[2]
           + mat[15] * src[3];
}

static __inline__ void
vec_mat_apply (FLOAT_TYPE *dst, FLOAT_TYPE *a, FLOAT_TYPE *b)
{
  vec_transform (&dst[0], a, &b[0]);
  vec_transform (&dst[4], a, &b[4]);
  vec_transform (&dst[8], a, &b[8]);
  vec_transform (&dst[12], a, &b[12]);
}

/* Transpose the rotation part of (an orthogonal) matrix only.  */

static __inline__ void
vec_transpose_rotation (FLOAT_TYPE *dst, FLOAT_TYPE *src)
{
  assert (dst != src);
  
  dst[0] = src[0];
  dst[1] = src[4];
  dst[2] = src[8];
  dst[3] = 0.0;
  
  dst[4] = src[1];
  dst[5] = src[5];
  dst[6] = src[9];
  dst[7] = 0.0;
  
  dst[8] = src[2];
  dst[9] = src[6];
  dst[10] = src[10];
  dst[11] = 0.0;
  
  dst[12] = 0.0;
  dst[13] = 0.0;
  dst[14] = 0.0;
  dst[15] = 1.0;
}

/* The angle between two vectors, between -PI and PI.  Result is as if
   A is rotated onto B, around the axis perpendicular to both.  If you want
   to know the axis too, pass non-NULL for AXB.  */

static __inline__ FLOAT_TYPE
vec_angle (FLOAT_TYPE *a, FLOAT_TYPE *b, FLOAT_TYPE *axb)
{
  FLOAT_TYPE a_cross_b[3];
  FLOAT_TYPE *a_cross_b_p = &a_cross_b[0];
  FLOAT_TYPE res;
  
  if (axb)
    a_cross_b_p = axb;
  
  vec_cross (a_cross_b_p, a, b);
  res = asinf (vec_length (a_cross_b_p) / (vec_length (a) * vec_length (b)));
  
  if (vec_dot (a, b) < 0)
    {
      if (res > 0)
        res = M_PI - res;
      else
        res = -M_PI - res;
    }

  return res;
}

#endif