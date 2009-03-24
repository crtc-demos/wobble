#include "transform.h"

/* stolen from Intel, tsk tsk
 * (eg http://www.intel.com/design/pentiumiii/sml/24504301.pdf)
 * Matrix inversion using Cramer's rule
 */
void transform_invertGLmatrix(GLfloat dst[16], GLfloat mat[16])
{
  GLfloat tmp[12];
  GLfloat src[16];
  GLfloat det;
  int i;
  
  /* transpose matrix */
  for (i = 0; i < 4; i++)
  {
    src[i] = mat[i*4];
    src[i + 4] = mat[i*4 + 1];
    src[i + 8] = mat[i*4 + 2];
    src[i + 12] = mat[i*4 + 3];
  }
  
  /* calculate pairs for first 8 elements (cofactors) */
  tmp[0] = src[10] * src[15];
  tmp[1] = src[11] * src[14];
  tmp[2] = src[9] * src[15];
  tmp[3] = src[11] * src[13];
  tmp[4] = src[9] * src[14];
  tmp[5] = src[10] * src[13];
  tmp[6] = src[8] * src[15];
  tmp[7] = src[11] * src[12];
  tmp[8] = src[8] * src[14];
  tmp[9] = src[10] * src[12];
  tmp[10] = src[8] * src[13];
  tmp[11] = src[9] * src[12];
  
  /* calculate first 8 elements (cofactors) */
  dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7]
         - tmp[1]*src[5] - tmp[2]*src[6] - tmp[5]*src[7];
  dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7]
         - tmp[0]*src[4] - tmp[7]*src[6] - tmp[8]*src[7];
  dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7]
         - tmp[3]*src[4] - tmp[6]*src[5] - tmp[11]*src[7];
  dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6]
         - tmp[4]*src[4] - tmp[9]*src[5] - tmp[10]*src[6];
  dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3]
         - tmp[0]*src[1] - tmp[3]*src[2] - tmp[4]*src[3];
  dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3]
         - tmp[1]*src[0] - tmp[6]*src[2] - tmp[9]*src[3];
  dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3]
         - tmp[2]*src[0] - tmp[7]*src[1] - tmp[10]*src[3];
  dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2]
         - tmp[5]*src[0] - tmp[8]*src[1] - tmp[11]*src[2];

  /* calculate pairs for second 8 elements (cofactors) */
  tmp[0] = src[2]*src[7];
  tmp[1] = src[3]*src[6];
  tmp[2] = src[1]*src[7];
  tmp[3] = src[3]*src[5];
  tmp[4] = src[1]*src[6];
  tmp[5] = src[2]*src[5];
  tmp[6] = src[0]*src[7];
  tmp[7] = src[3]*src[4];
  tmp[8] = src[0]*src[6];
  tmp[9] = src[2]*src[4];
  tmp[10] = src[0]*src[5];
  tmp[11] = src[1]*src[4];

  /* calculate second 8 elements (cofactors) */
  dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15]
         - tmp[1]*src[13] - tmp[2]*src[14] - tmp[5]*src[15];
  dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15]
         - tmp[0]*src[12] - tmp[7]*src[14] - tmp[8]*src[15];
  dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15]
          - tmp[3]*src[12] - tmp[6]*src[13] - tmp[11]*src[15];
  dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14]
          - tmp[4]*src[12] - tmp[9]*src[13] - tmp[10]*src[14];
  dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9]
          - tmp[4]*src[11] - tmp[0]*src[9] - tmp[3]*src[10];
  dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10]
          - tmp[6]*src[10] - tmp[9]*src[11] - tmp[1]*src[8];
  dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8]
          - tmp[10]*src[11] - tmp[2]*src[8] - tmp[7]*src[9];
  dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9]
          - tmp[8]*src[9] - tmp[11]*src[10] - tmp[5]*src[8];

  /* calculate determinant */
  det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

  /* calculate matrix inverse */
  det = 1/det;
  for (i = 0; i < 16; i++)
    dst[i] *= det;
}

void transform_transposeGLmatrix(GLfloat dest[16], GLfloat src[16])
{
  int i, j;
  
  for (j=0; j<4; j++)
  {
    for (i=0; i<4; i++)
    {
      dest[j*4+i] = src[i*4+j];
    }
  }
}

/* GL-equivalent matrix transformation */
void transform_pointGL(GLfloat dest[4], GLfloat mat[16], GLfloat src[4])
{
  /* making copies of these stops them from being reloaded */
  double x=src[0], y=src[1], z=src[2], w = src[3];
  
  dest[0] = mat[0] * x
          + mat[1] * y
          + mat[2] * z
          + mat[3] * w;

  dest[1] = mat[4] * x
          + mat[5] * y
          + mat[6] * z
          + mat[7] * w;

  dest[2] = mat[8] * x
          + mat[9] * y
          + mat[10] * z
          + mat[11] * w;

  dest[3] = mat[12] * x
          + mat[13] * y
          + mat[14] * z
          + mat[15] * w;
}
