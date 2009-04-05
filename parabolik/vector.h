#ifndef VECTOR_H
#define VECTOR_H 1

#ifndef FLOAT_TYPE
#define FLOAT_TYPE float
#endif

static __inline__ FLOAT_TYPE
vec_dot (FLOAT_TYPE *a, FLOAT_TYPE *b)
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static __inline__ void
vec_normalize (FLOAT_TYPE *dst, FLOAT_TYPE *in)
{
  FLOAT_TYPE factor = fsqrt (in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
  
  if (factor != 0.0)
    factor = 1.0 / factor;
  
  dst[0] = in[0] * factor;
  dst[1] = in[1] * factor;
  dst[2] = in[2] * factor;
}

static __inline__ FLOAT_TYPE
vec_length (FLOAT_TYPE *vec)
{
  return fsqrt (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

static __inline__ void
vec_cross (FLOAT_TYPE *dst, FLOAT_TYPE *a, FLOAT_TYPE *b)
{
  FLOAT_TYPE dstc[3];
  
  dstc[0] = a[1] * b[2] - a[2] * b[1];
  dstc[1] = a[2] * b[0] - a[0] * b[2];
  dstc[2] = a[0] * b[1] - a[1] * b[0];
  dst[0] = dstc[0];
  dst[1] = dstc[1];
  dst[2] = dstc[2];
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

#endif
