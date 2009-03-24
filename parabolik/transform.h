#ifndef INVERT_H
#define INVERT_H 1

#include <GL/gl.h>

extern void transform_invertGLmatrix(GLfloat dst[16], GLfloat src[16]);
extern void transform_transposeGLmatrix(GLfloat dst[16], GLfloat src[16]);
void transform_pointGL(GLfloat dest[4], GLfloat mat[16], GLfloat src[4]);

#endif
