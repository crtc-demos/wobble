#ifndef OBJECT_H
#define OBJECT_H 1

#include <kos.h>

#include "vector.h"
#include "fakephong.h"
#include "envmap_dual_para.h"
#include "bumpmap.h"
#include "vertex_fog.h"
#include "viewpoint.h"
#include "lighting.h"
#include "colour.h"

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
  unsigned int txr_idx;
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

typedef struct object
{
  strip *striplist;
  
  colour ambient;
  colour pigment;
  
  /* Whether to clip this object.  */
  int clip;
  
  /* Cached max. length of strip.  */
  int max_strip_length;
  
  /* Poly is textured.  */
  int textured;
  
  /* Render object with specular highlight if non-NULL.  */
  fakephong_info *fake_phong;
  
  /* Render object with dual-paraboloid environment map if non-NULL.  */
  envmap_dual_para_info *env_map;
  
  /* Render using bump map if non-NULL.  */
  bumpmap_info *bump_map;
  
  /* Render with vertex fog if non-NULL.  */
  vertexfog_info *vertex_fog;
} object;

typedef struct object_orientation
{
  matrix_t *modelview;
  matrix_t *normal_xform;
  int dirty;
} object_orientation;

extern object *object_create_default (strip *strips);
extern void object_set_ambient (object *obj, int r, int g, int b);
extern void object_set_pigment (object *obj, int r, int g, int b);
extern void object_set_clipping (object *obj, int clip);
extern void object_set_all_textures (object *obj, pvr_ptr_t txr,
				     unsigned int xsize, unsigned int ysize,
				     unsigned int format);
extern void object_render_immediate (viewpoint *view, object *,
				     object_orientation *, lighting *, int);
extern void object_render_untextured_phong (viewpoint *view, object *,
					    object_orientation *, lighting *,
					    int);
extern void object_render_deferred (viewpoint *view, object *,
				    object_orientation *, lighting *);
extern strip *strip_cons (strip *prev, unsigned int length,
			  unsigned int alloc_bits);

#undef USE_DMA

/* This has no business being here... */
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(X) (sizeof (X) / sizeof (X[0]))
#endif

#endif
