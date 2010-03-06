#ifndef ENVMAP_DUAL_PARA_H
#define ENVMAP_DUAL_PARA_H 1

#include <kos.h>

typedef struct
{
  pvr_ptr_t front_txr;
  unsigned int front_txr_fmt;
  pvr_ptr_t back_txr;
  unsigned int back_txr_fmt;
  unsigned int xsize;
  unsigned int ysize;
} envmap_dual_para_info;

typedef struct
{
  float texc_f[3];
  float texc_b[3];
} envmap_dual_para_vertex_info;

struct vertex_attrs;

extern void envmap_dual_para_texcoords (float *texc_f, float *texc_b,
					float x_vertex[3], float x_normal[3],
					float c_eyepos[3], matrix_t invcamera);

extern int envmap_classify_triangle (float tri[3][3], int clockwise,
				     struct vertex_attrs *attrs);

#endif
