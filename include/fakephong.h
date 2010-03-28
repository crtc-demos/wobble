#ifndef FAKEPHONG_H
#define FAKEPHONG_H 1

#include <kos.h>

typedef struct
{
  pvr_ptr_t highlight;
  unsigned int xsize;
  unsigned int ysize;
  int intensity;
} fakephong_info;

typedef struct
{
  float texc[2];
  float transformed_z;
} fakephong_vertex_info;

struct vertex_attrs;
struct colour;

extern void fakephong_highlight_texture (pvr_ptr_t highlight,
					 unsigned int xsize,
					 unsigned int ysize, float hardness);

extern unsigned long lightsource_diffuse (const float *, const float *,
					  const struct colour *,
					  const struct colour *,
					  const float *);

extern void lightsource_fake_phong (float *, float *, const float *,
				    const float *, const float *, matrix_t *,
				    struct vertex_attrs *, unsigned int);

extern int fakephong_classify_triangle (float[3][3], int,
					struct vertex_attrs *);

extern int fakephong_classify_triangle_1pass (float[3][3], int,
					      struct vertex_attrs *);

#endif
