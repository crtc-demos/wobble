#ifndef OBJECT_H
#define OBJECT_H 1

#include <kos.h>

#include "vector.h"
#include "fakephong.h"
#include "envmap_dual_para.h"
#include "bumpmap.h"
#include "vertex_fog.h"

typedef float vector[3];

typedef struct vertex_attrs
{
  fakephong_vertex_info fakephong;
  envmap_dual_para_vertex_info env_map;
} vertex_attrs;

/* Attributes for a whole strip.  */
typedef struct
{
  /* Not sure if we want these here?  */
  pvr_ptr_t texture;
  unsigned int xsize;
  unsigned int ysize;
  unsigned int txr_fmt;
  /* Not 100% sure about this either.  */
  float uv_orient[3];
} strip_attrs;

#define ALLOC_GEOMETRY		(1 << 0)
#define ALLOC_NORMALS		(1 << 1)
#define ALLOC_TEXCOORDS		(1 << 2)
#define ALLOC_VERTEXATTRS	(1 << 3)

typedef struct strip
{
  float (*start)[][3];
  float (*normals)[][3];
  float (*texcoords)[][2];
  int length;
  /* Inverse is 1 for inverted strips.  To render, e.g., send first vertex
     twice.  */
  int inverse;
  vertex_attrs *v_attrs;
  strip_attrs *s_attrs;
  struct strip *next;
} strip;

typedef struct colour
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} colour;

typedef struct object
{
  strip *striplist;
  
  colour ambient;
  colour pigment;
  
  /* Whether to clip this object.  */
  int clip;
  
  /* Cached max. length of strip.  */
  int max_strip_length;
  
  /* Plain textured, no lighting or fancy stuff.  */
  int plain_textured;
  
  /* Render object with specular highlight if non-NULL.  */
  fakephong_info *fake_phong;
  
  /* Render object with dual-paraboloid environment map if non-NULL.  */
  envmap_dual_para_info *env_map;
  
  /* Render using bump map if non-NULL.  */
  bumpmap_info *bump_map;
  
  /* Render with vertex fog if non-NULL.  */
  vertexfog_info *vertex_fog;
} object;

typedef struct viewpoint
{
  matrix_t *projection;
  matrix_t *camera;
  matrix_t *inv_camera_orientation;
  float eye_pos[3];
  float near;
} viewpoint;

typedef struct object_orientation
{
  matrix_t *modelview;
  matrix_t *normal_xform;
} object_orientation;

typedef struct lighting
{
  float light0_pos[3];
  float light0_up[3];
  float light0_pos_xform[3];
  float light0_up_xform[3];
} lighting;

extern object *object_create_default (strip *strips);
extern void object_set_ambient (object *obj, int r, int g, int b);
extern void object_set_pigment (object *obj, int r, int g, int b);
extern void object_set_clipping (object *obj, int clip);
extern void object_set_all_textures (object *obj, pvr_ptr_t txr,
				     unsigned int xsize, unsigned int ysize,
				     unsigned int format);
extern void object_render_immediate (viewpoint *view, object *,
				     object_orientation *, lighting *, int);
extern strip *strip_cons (strip *prev, unsigned int length,
			  unsigned int alloc_bits);

#endif
