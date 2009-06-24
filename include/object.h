#ifndef OBJECT_H
#define OBJECT_H 1

#include "vector.h"
#include "fakephong.h"
#include "envmap_dual_para.h"

typedef float vector[3];

typedef struct vertex_attrs
{
  fakephong_vertex_info fakephong;
  envmap_dual_para_vertex_info env_map;
} vertex_attrs;

typedef struct strip
{
  float (*start)[][3];
  float (*normals)[][3];
  int length;
  /* Inverse is 1 for inverted strips.  To render, e.g., send first vertex
     twice.  */
  int inverse;
  vertex_attrs *attrs;
  struct strip *next;
} strip;

typedef struct colour
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} colour;

typedef struct
{
  strip *striplist;
  
  colour ambient;
  colour pigment;
  
  /* Render object with specular highlight if non-NULL.  */
  fakephong_info *fake_phong;
  
  /* Render object with dual-paraboloid environment map if non-NULL.  */
  envmap_dual_para_info *env_map;
  
  /* Render using bump map if non-NULL.  */
  void *bump_map;
} object;

extern object *object_create_default (strip *strips);
extern void object_set_ambient (object *obj, int r, int g, int b);
extern void object_set_pigment (object *obj, int r, int g, int b);
extern void object_render_immediate (const object *, int, matrix_t mv,
				     matrix_t nx, matrix_t pr, matrix_t cam,
				     matrix_t icam, const float *eye,
				     const float *lpos, const float *lup);

#endif
