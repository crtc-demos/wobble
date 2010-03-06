/* The object rendering pipeline should look like this (based on a
   badly-remembered version of the OpenGL pipeline):
   
     - Modelview transform
     - [texture UV generation] *
     - Lighting *
     - Clipping (Z near only, using restripper)
     - Perspective transform
     - [Screen transform -- probably composed with perspective xform]
   
   For (*), we also need e.g. the camera view "component" of the modelview
   matrix, as well as the inverse, rotation-only part of same.
   
   We'll generally need to send objects to the graphics hardware more than once:
   we probably want to cache the transformed vertices as much as possible
   between such passes. Might it be possible to send different poly headers, but
   use the same vertex data (sometimes)?
   
   (Otherwise, for DMA-based rendering, output two copies of transformed
   vertices, for e.g. opaque and transparent polys, so we don't do all the
   calculations twice.)
*/

#include <kos.h>
#include <stdlib.h>

#include "restrip.h"
#include "object.h"
#include "perlin.h"
#include "perlin-3d.h"

/*
#ifndef SLOW_PVR_PRIM
#define pvr_prim (DATA, SIZE) sq_cpy ((void *) PVR_TA_INPUT, (DATA), (SIZE))
#endif
*/

object *
object_create_default (strip *strips)
{
  object *newobj = malloc (sizeof (object));
  
  newobj->striplist = strips;
  newobj->ambient.r = 32;
  newobj->ambient.g = 32;
  newobj->ambient.b = 32;
  newobj->pigment.r = 192;
  newobj->pigment.g = 192;
  newobj->pigment.b = 192;
  newobj->clip = 0;
  newobj->max_strip_length = -1;
  newobj->plain_textured = 0;
  newobj->fake_phong = NULL;
  newobj->env_map = NULL;
  newobj->bump_map = NULL;
  newobj->vertex_fog = NULL;
  
  return newobj;
}

void
object_set_ambient (object *obj, int r, int g, int b)
{
  obj->ambient.r = r;
  obj->ambient.g = g;
  obj->ambient.b = b;
}

void
object_set_pigment (object *obj, int r, int g, int b)
{
  obj->pigment.r = r;
  obj->pigment.g = g;
  obj->pigment.b = b;
}

void
object_set_clipping (object *obj, int clipping)
{
  obj->clip = clipping;
}

/* Set all textures (in strip attribute block) to the same one.  Mostly for
   debugging.  */

void
object_set_all_textures (object *obj, pvr_ptr_t texture, unsigned int xsize,
			 unsigned int ysize)
{
  strip *str;
  
  for (str = obj->striplist; str; str = str->next)
    {
      if (str->s_attrs == NULL)
        str->s_attrs = calloc (1, sizeof (strip_attrs));

      str->s_attrs->texture = texture;
      str->s_attrs->xsize = xsize;
      str->s_attrs->ysize = ysize;
    }
}

static unsigned int
max_strip_length (object *obj)
{
  unsigned int max_length = 0;
  strip *str;
  
  if (obj->max_strip_length != -1)
    return obj->max_strip_length;
  
  for (str = obj->striplist; str; str = str->next)
    if (str->length > max_length)
      max_length = str->length;

  obj->max_strip_length = max_length;

  return max_length;
}

static strip *stripbuf = NULL;
static unsigned int capacity = 0;
static strip *clipped_stripbuf = NULL;
static unsigned int clipped_capacity = 0;

/* Non-textured, packed colour.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 d1, d2;
  uint32 argb;
  uint32 d3;
} pvr_vertex_type0_t;

/* Non-textured, floating colour.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  float a, r, g, b;
} pvr_vertex_type1_t;

/* Non-textured, intensity.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 d1, d2;
  uint32 intens;
  uint32 d3;
} pvr_vertex_type2_t;

/* Textured, packed colour.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  float u, v;
  uint32 argb;
  uint32 oargb;
} pvr_vertex_type3_t;

/* Textured, packed colour, 16 bit UV.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 uv;
  uint32 d1;
  uint32 argb;
  uint32 oargb;
} pvr_vertex_type4_t;

/* Textured, floating colour.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  float u, v;
  uint32 d1, d2;
  float a, r, g, b;
  float oa, or, og, ob;
} pvr_vertex_type5_t;

/* Textured, floating colour, 16 bit UV.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 uv;
  uint32 d1, d2, d3;
  float a, r, g, b;
  float oa, or, og, ob;
} pvr_vertex_type6_t;

/* Textured, intensity.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  float u, v;
  float intens;
  float ointens;
} pvr_vertex_type7_t;

/* Textured, intensity, 16 bit UV.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 uv;
  uint32 d1;
  float intens;
  float ointens;
} pvr_vertex_type8_t;

/* Non-textured, packed colour, affected by modifier volume.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 argb0;
  uint32 argb1;
  uint32 d1, d2;
} pvr_vertex_type9_t;

/* Non-textured, intensity, affected by modifier volume.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  float intens0;
  float intens1;
  uint32 d1, d2;
} pvr_vertex_type10_t;

/* Textured, packed colour, affected by modifier volume.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  float u0, v0;
  uint32 argb0;
  uint32 oargb0;
  float u1, v1;
  uint32 argb1;
  uint32 oargb1;
  uint32 d1, d2, d3, d4;
} pvr_vertex_type11_t;

/* Textured, packed colour, 16 bit UV, affected by modifier volume.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 uv0;
  uint32 d1;
  uint32 argb0;
  uint32 oargb0;
  uint32 uv1;
  uint32 d2;
  uint32 argb1;
  uint32 oargb1;
  uint32 d3, d4, d5, d6;
} pvr_vertex_type12_t;

/* Textured, intensity, affected by modifier volume.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  float u0, v0;
  float intens0;
  float ointens0;
  float u1, v1;
  float intens1;
  float ointens1;
  uint32 d1, d2, d3, d4;
} pvr_vertex_type13_t;

/* Textured, intensity, 16 bit UV, affected by modifier volume.  */

typedef struct {
  uint32 flags;
  float x, y, z;
  uint32 uv0;
  uint32 d1;
  float intens0;
  float ointens0;
  uint32 uv1;
  uint32 d2;
  float intens1;
  float ointens1;
  uint32 d3, d4, d5, d6;
} pvr_vertex_type14_t;

/* Non-textured sprite.  */

typedef struct {
  uint32 flags;
  float ax, ay, az;
  float bx, by, bz;
  float cx, cy, cz;
  float dx, dy;
  uint32 d1, d2, d3, d4;
} pvr_vertex_type15_t;

/* Textured sprite.  */

typedef struct {
  uint32 flags;
  float ax, ay, az;
  float bx, by, bz;
  float cx, cy, cz;
  float dx, dy;
  uint32 d1;
  uint32 auv;
  uint32 buv;
  uint32 cuv;
} pvr_vertex_type16_t;

/* Shadow volume.  */

typedef struct {
  uint32 flags;
  float ax, ay, az;
  float bx, by, bz;
  float cx, cy, cz;
  uint32 d1, d2, d3, d4, d5, d6;
} pvr_vertex_type17_t;

#define ALPHA_LIST PVR_LIST_TR_POLY
//#define ALPHA_LIST PVR_LIST_PT_POLY

typedef struct mem_pool {
  char *buffer;
  unsigned int used;
  unsigned int capacity;
  struct mem_pool *next;
} mem_pool;

static mem_pool pool = {
  .buffer = NULL,
  .used = 0,
  .capacity = 0,
  .next = NULL
};

static mem_pool *current_pool = NULL;

#define POOL_PART_SIZE	(64 * 1024)

void
pool_clear (void)
{
  if (pool.buffer == NULL)
    {
      /* Make an initial pool.  */
      pool.buffer = malloc (POOL_PART_SIZE);
      pool.used = 0;
      pool.capacity = POOL_PART_SIZE;
      pool.next = NULL;
    }
  else
    {
      mem_pool *iter;
  
      for (iter = &pool; iter; iter = iter->next)
	iter->used = 0;
    }

  current_pool = &pool;
}

void *
pool_alloc (unsigned int amt)
{
retry:
  if (current_pool->used + amt > current_pool->capacity)
    {
      mem_pool *newpart;
      unsigned int newsize = (amt > POOL_PART_SIZE) ? amt : POOL_PART_SIZE;
      
      if (current_pool->next != NULL)
        {
	  current_pool = current_pool->next;
          goto retry;
	}
      
      newpart = malloc (sizeof (mem_pool));
      
      fprintf (stderr, "Pool allocator: allocating new part (size %u)\n",
	       newsize);
      
      current_pool->next = newpart;
      
      newpart->buffer = malloc (newsize);
      newpart->used = amt;
      newpart->capacity = newsize;
      newpart->next = NULL;
      
      current_pool = newpart;
      
      return &newpart->buffer[0];
    }
  else
    {
      char *block = &current_pool->buffer[current_pool->used];

      current_pool->used += amt;
      
      return block;
    }
}

static strip *strip_cons_pool (strip *prev, unsigned int length,
			       unsigned int alloc_bits);

static float near_plane;

static int
triangle_clipping_classifier (float tri[3][3], int clockwise,
			      vertex_attrs *attrs)
{
  if (tri[0][2] < near_plane
      && tri[1][2] < near_plane
      && tri[2][2] < near_plane)
    return 0;
  else if (tri[0][2] >= near_plane
	   && tri[1][2] >= near_plane
	   && tri[2][2] >= near_plane)
    return -1;
  else
    return 1;
}

float amt = 0.0;

#if 1
#define PREFETCH(X)
#else
#define PREFETCH(X) __asm__ __volatile__ ("pref @%0" : : "r" (X))
#endif


/* NORMAL_XFORM is the transpose-inverse of the rotation part of the MODELVIEW
   matrix.  */

/* This is horribly misorganised, but it's only meant to be a prototype.
   Needs fixing!  */

void
object_render_immediate (viewpoint *view, object *obj,
			 object_orientation *obj_orient, lighting *lights,
			 int pass)
{
  unsigned int strip_max = max_strip_length (obj), i;
  float (*trans)[][3], (*trans_norm)[][3];
  strip *str;
  float c_eyepos[3];
  vertex_attrs *attr_buf;
  
  pool_clear ();
  
  attr_buf = pool_alloc (sizeof (vertex_attrs) * strip_max);
  
  if (capacity == 0)
    {
      capacity = 10;
      stripbuf = malloc (sizeof (strip) * capacity);
    }
  
  if (clipped_capacity == 0)
    {
      clipped_capacity = 10;
      clipped_stripbuf = malloc (sizeof (strip) * capacity);
    }

  if (obj->env_map)
    vec_transform3_fipr (&c_eyepos[0], (float *) view->camera,
			 &view->eye_pos[0]);

  trans = pool_alloc (sizeof (float) * 3 * strip_max);
  trans_norm = pool_alloc (sizeof (float) * 3 * strip_max);

  for (str = obj->striplist; str; str = str->next)
    {
      strip transformed_strip;
      strip *use_strip;

      mat_load (obj_orient->modelview);

      for (i = 0; i < str->length; i++)
        {
	  float *vec = &(*str->start)[i][0], *outvec = &(*trans)[i][0];
	  float x = vec[0], y = vec[1], z = vec[2], w = 1.0;
	  PREFETCH (&vec[3]);
	  mat_trans_nodiv (x, y, z, w);
	  outvec[0] = x;
	  outvec[1] = y;
	  outvec[2] = z;
	}

      mat_load (obj_orient->normal_xform);

      for (i = 0; i < str->length; i++)
        {
	  float *norm = &(*str->normals)[i][0], *outnorm = &(*trans_norm)[i][0];
	  float x = norm[0], y = norm[1], z = norm[2], w = 1.0;
	  PREFETCH (&norm[3]);
	  mat_trans_nodiv (x, y, z, w);
	  outnorm[0] = x;
	  outnorm[1] = y;
	  outnorm[2] = z;
	}

      transformed_strip.start = trans;
      transformed_strip.length = str->length;
      transformed_strip.normals = trans_norm;
      transformed_strip.texcoords = str->texcoords;
      transformed_strip.inverse = str->inverse;
      transformed_strip.v_attrs = attr_buf;
      transformed_strip.s_attrs = str->s_attrs;
      transformed_strip.next = NULL;

      if (obj->clip)
        {
	  strip *strip_starts[2] = { 0, 0 };
	  strip *strip_ends[2] = { 0, 0 };
	  strip *cstr;
	  strip *clipped_output;
	  unsigned int alloc_mask = 0;
	  
	  /* A sneaky global!  */
	  near_plane = view->near;
	  
	  /* Split strips into 0 (fully visible) or 1 (partly visible,
	     clipped).  Drop fully invisible triangles/strips on the floor.  */
	  restrip_list (&transformed_strip, triangle_clipping_classifier,
			strip_starts, strip_ends, &clipped_stripbuf,
			&clipped_capacity);

	  /* Glue clipped strips onto the start of the fully-visible strip
	     list.  */
	  clipped_output = strip_starts[0];

	  for (cstr = strip_starts[1]; cstr; cstr = cstr->next)
	    {
	      if (cstr->length < 3)
	        continue;

	      alloc_mask = cstr->start ? ALLOC_GEOMETRY : 0;
	      alloc_mask |= cstr->normals ? ALLOC_NORMALS : 0;
	      alloc_mask |= cstr->texcoords ? ALLOC_TEXCOORDS : 0;
	      alloc_mask |= cstr->v_attrs ? ALLOC_VERTEXATTRS : 0;

	      for (i = 0; i < cstr->length - 2; i++)
	        {
		  int visible_points, output_length;
		  int pt_visible[3];
		  unsigned int j, outidx = 0;
		  float points[4][3], normals[4][3], texcs[4][2];
		  
		  pt_visible[0] = (*cstr->start)[i][2] < view->near;
		  pt_visible[1] = (*cstr->start)[i + 1][2] < view->near;
		  pt_visible[2] = (*cstr->start)[i + 2][2] < view->near;
		  
		  visible_points = pt_visible[0] + pt_visible[1]
				   + pt_visible[2];
		  
		  if (visible_points == 1)
		    output_length = 3;
		  else if (visible_points == 2)
		    output_length = 4;
		  else
		    abort ();
		  
		  clipped_output = strip_cons_pool (clipped_output,
		    output_length, alloc_mask);
		  
		  clipped_output->inverse = cstr->inverse ^ (i & 1);
		  clipped_output->s_attrs = cstr->s_attrs;
		  
		  for (j = 0; j < 3; j++)
		    {
		      unsigned int next = (j == 2) ? 0 : j + 1;

		      if (pt_visible[j])
		        {
			  memcpy (&points[outidx][0], &(*cstr->start)[i + j][0],
				  3 * sizeof (float));
			  memcpy (&normals[outidx][0],
				  &(*cstr->normals)[i + j][0],
				  3 * sizeof (float));
			  if (cstr->texcoords)
			    memcpy (&texcs[outidx][0],
				    &(*cstr->texcoords)[i + j][0],
				    2 * sizeof (float));
			  outidx++;
			}
		      
		      if ((pt_visible[j] && !pt_visible[next])
			  || (!pt_visible[j] && pt_visible[next]))
		        {
			  /* Interpolate and be damned!  */
			  float param
			    = (view->near - (*cstr->start)[i + j][2])
			      / ((*cstr->start)[i + next][2]
				 - (*cstr->start)[i + j][2]);
			  
			  vec_interpolate (&points[outidx][0],
					   &(*cstr->start)[i + j][0],
					   &(*cstr->start)[i + next][0], param);
			  vec_interpolate (&normals[outidx][0],
					   &(*cstr->normals)[i + j][0],
					   &(*cstr->normals)[i + next][0],
					   param);
			  if (cstr->texcoords)
			    vec_interpolate2 (&texcs[outidx][0],
					      &(*cstr->texcoords)[i + j][0],
					      &(*cstr->texcoords)[i + next][0],
					      param);
			  outidx++;
			}
		    }
		  
		  assert (outidx == output_length);
		  
		  switch (outidx)
		    {
		    case 3:
		      /* Easy case: we have a single triangle, in the same
		         order as the original strip.  */
		      memcpy (&(*clipped_output->start)[0][0],
			      points, 3 * 3 * sizeof (float));
		      memcpy (&(*clipped_output->normals)[0][0],
			      normals, 3 * 3 * sizeof (float));
		      if (clipped_output->texcoords)
		        memcpy (&(*clipped_output->texcoords)[0][0],
				texcs, 3 * 2 * sizeof (float));
		      break;
		    
		    case 4:
		      /* We have four points going round a circle.  Need to
			 make them zig-zag in the normal strip style
			 instead.  */
		      {
		        unsigned int order[] = {0, 1, 3, 2};

			for (j = 0; j < 4; j++)
			  {
			    memcpy (&(*clipped_output->start)[j][0],
				    &points[order[j]][0], 3 * sizeof (float));
			    memcpy (&(*clipped_output->normals)[j][0],
				    &normals[order[j]][0], 3 * sizeof (float));
			    if (clipped_output->texcoords)
			      memcpy (&(*clipped_output->texcoords)[j][0],
				      &texcs[order[j]][0], 2 * sizeof (float));
			  }
		      }
		      break;
		    
		    default:
		      abort ();
		    }
		}
	    }
	  
	  use_strip = clipped_output;
	}
      else
        use_strip = &transformed_strip;

      if (obj->env_map)
        {
	  /* Dual-paraboloid environment mapping.  */
	  strip *strip_starts[3] = { 0, 0, 0 };
	  strip *strip_ends[3] = { 0, 0, 0 };
	  pvr_poly_cxt_t cxt;
	  pvr_poly_hdr_t hdr;
	  strip *str_out, *iter;
	  pvr_vertex_t vert;
	  int class;

	  vert.argb = PVR_PACK_COLOR (1.0f, 1.0f, 1.0f, 1.0f);
	  vert.oargb = 0;

	  for (iter = use_strip; iter; iter = iter->next)
	    for (i = 0; i < iter->length; i++)
	      envmap_dual_para_texcoords
		(&iter->v_attrs[i].env_map.texc_f[0],
		 &iter->v_attrs[i].env_map.texc_b[0],
		 (*iter->start)[i], (*iter->normals)[i],
		 &c_eyepos[0], *(view->inv_camera_orientation));

	  restrip_list (use_strip, envmap_classify_triangle,
			strip_starts, strip_ends, &stripbuf, &capacity);

	  mat_load (view->projection);

	  /* This way of doing things wastes a lot of time here (due to
	     recalculation of above), and it's not really necessary.  Only
	     temporary until we get DMA working.  */

          if (pass == 0)
	    {
	      /* Render front strips.  */
	      pvr_poly_cxt_txr (&cxt, PVR_LIST_OP_POLY,
				obj->env_map->front_txr_fmt,
				obj->env_map->xsize, obj->env_map->ysize,
				obj->env_map->front_txr, PVR_FILTER_BILINEAR);

	      pvr_poly_compile (&hdr, &cxt);
	      
	      /* Render both front and front-and-back strips (class 0 and
	         2).  */
	      for (class = 0; class <= 2; class += 2)
		for (str_out = strip_starts[class]; str_out;
		     str_out = str_out->next)
	          {
		    if (str_out->length < 3)
		      continue;

		    pvr_prim (&hdr, sizeof (hdr));

		    for (i = 0; i < str_out->length; i++)
		      {
			float *invec = &(*str_out->start)[i][0];
			float x, y, z;
			int last = (i == str_out->length - 1);

			x = invec[0];
			y = invec[1];
			z = invec[2];
			mat_trans_single (x, y, z);
			
			vert.flags = (last) ? PVR_CMD_VERTEX_EOL
					    : PVR_CMD_VERTEX;
			vert.x = x;
			vert.y = y;
			vert.z = z;
			vert.u = str_out->v_attrs[i].env_map.texc_f[0];
			vert.v = str_out->v_attrs[i].env_map.texc_f[1];
			pvr_prim (&vert, sizeof (vert));
			if (i == 0 && str_out->inverse)
		          pvr_prim (&vert, sizeof (vert));
		      }
		  }

	      pvr_poly_cxt_txr (&cxt, PVR_LIST_OP_POLY,
				obj->env_map->back_txr_fmt,
				obj->env_map->xsize, obj->env_map->ysize,
				obj->env_map->back_txr, PVR_FILTER_BILINEAR);
			
	      /* Render back-only strips.  */
	      class = 1;
	    }
	  else
	    {
	      pvr_poly_cxt_txr (&cxt, ALPHA_LIST,
	      			obj->env_map->back_txr_fmt, obj->env_map->xsize,
				obj->env_map->ysize, obj->env_map->back_txr,
				PVR_FILTER_BILINEAR);
	      cxt.txr.uv_clamp = PVR_UVCLAMP_UV;
	      cxt.blend.src = PVR_BLEND_SRCALPHA;
	      cxt.blend.dst = PVR_BLEND_INVSRCALPHA;
	      cxt.txr.env = PVR_TXRENV_MODULATE;
	      cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
	      /* Render front-and-back strips (back part, with alpha).  */
	      class = 2;
	    }

	  pvr_poly_compile (&hdr, &cxt);

	  for (str_out = strip_starts[class]; str_out; str_out = str_out->next)
	    {
	      if (str_out->length < 3)
		continue;

	      pvr_prim (&hdr, sizeof (hdr));

	      for (i = 0; i < str_out->length; i++)
		{
		  float *invec = &(*str_out->start)[i][0];
		  float x, y, z;
		  int last = (i == str_out->length - 1);

		  x = invec[0];
		  y = invec[1];
		  z = invec[2];
		  mat_trans_single (x, y, z);

		  vert.flags = (last) ? PVR_CMD_VERTEX_EOL
				      : PVR_CMD_VERTEX;
		  vert.x = x;
		  vert.y = y;
		  vert.z = z;
		  vert.u = str_out->v_attrs[i].env_map.texc_b[0];
		  vert.v = str_out->v_attrs[i].env_map.texc_b[1];
		  pvr_prim (&vert, sizeof (vert));
		  if (i == 0 && str_out->inverse)
		    pvr_prim (&vert, sizeof (vert));
		}
	    }
	}

      /* First pass for bump mapping, fake phong -- for now, plain
         gouraud-shaded polygons only.  */
      if (pass == 0 && /*(obj->bump_map || obj->fake_phong) &&*/ !obj->env_map
	  && !obj->vertex_fog && !obj->plain_textured)
        {
	  /* First pass, dot-product lighting.  */
	  pvr_poly_cxt_t cxt;
	  pvr_poly_hdr_t hdr;
	  pvr_vertex_t vert;
	  
	  pvr_poly_cxt_col (&cxt, PVR_LIST_OP_POLY);
	  
	  if (obj->bump_map)
	    cxt.txr.env = PVR_TXRENV_DECAL;
	  
	  pvr_poly_compile (&hdr, &cxt);

	  pvr_prim (&hdr, sizeof (hdr));
	  
	  vert.oargb = 0;
	  vert.argb = PVR_PACK_COLOR (1.0f, 1.0f, 1.0f, 1.0f);

	  mat_load (view->projection);

	  for (; use_strip; use_strip = use_strip->next)
	    for (i = 0; i < use_strip->length; i++)
	      {
		float *invec = &(*use_strip->start)[i][0];
		float x, y, z;
		int last = (i == use_strip->length - 1);

		//PREFETCH (&(*use_strip->start)[i + 2][0]);

		x = invec[0];
		y = invec[1];
		z = invec[2];
		mat_trans_single (x, y, z);

		//PREFETCH (&(*use_strip->normals)[i + 2][0]);

		vert.flags = (last) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
		vert.argb = lightsource_diffuse (&(*use_strip->start)[i][0],
		  &(*use_strip->normals)[i][0], &obj->ambient, &obj->pigment,
		  &lights->light0_pos_xform[0]);
		vert.x = x;
		vert.y = y;
		vert.z = z;
		vert.u = 0;
		vert.v = 0;
		pvr_prim (&vert, sizeof (vert));
		if (i == 0 && use_strip->inverse)
		  pvr_prim (&vert, sizeof (vert));
	      }
	}

      if (pass == 0 && obj->plain_textured)
        {
	  pvr_poly_cxt_t cxt;
	  pvr_poly_hdr_t hdr;
	  pvr_vertex_t vert;
	  	  
	  vert.oargb = 0;
	  vert.argb = PVR_PACK_COLOR (1.0f, 1.0f, 1.0f, 1.0f);

	  mat_load (view->projection);

	  for (; use_strip; use_strip = use_strip->next)
	    {
	      pvr_poly_cxt_txr (&cxt, PVR_LIST_OP_POLY,
		PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
		use_strip->s_attrs->xsize, use_strip->s_attrs->ysize,
		use_strip->s_attrs->texture, PVR_FILTER_BILINEAR);
	  	  
	      pvr_poly_compile (&hdr, &cxt);

	      pvr_prim (&hdr, sizeof (hdr));

	      for (i = 0; i < use_strip->length; i++)
		{
		  float *invec = &(*use_strip->start)[i][0];
		  float *texc = &(*use_strip->texcoords)[i][0];
		  float x, y, z;
		  int last = (i == use_strip->length - 1);

		  //PREFETCH (&(*use_strip->start)[i + 2][0]);

		  x = invec[0];
		  y = invec[1];
		  z = invec[2];
		  mat_trans_single (x, y, z);

		  //PREFETCH (&(*use_strip->normals)[i + 2][0]);

		  vert.flags = (last) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
		  vert.x = x;
		  vert.y = y;
		  vert.z = z;
		  vert.u = texc[0];
		  vert.v = texc[1];
		  pvr_prim (&vert, sizeof (vert));
		  if (i == 0 && use_strip->inverse)
		    pvr_prim (&vert, sizeof (vert));
		}
	    }
	}

      /* First pass for vertex fog (experimental).  */
      if (pass == 1 && obj->vertex_fog)
        {
	  pvr_poly_cxt_t cxt;
	  pvr_poly_hdr_t hdr;
	  pvr_vertex_type3_t vert;
	  
	  pvr_poly_cxt_txr (&cxt, PVR_LIST_TR_POLY,
			    PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
			    obj->vertex_fog->w, obj->vertex_fog->h,
			    obj->vertex_fog->texture, PVR_FILTER_BILINEAR);
	  
	  cxt.gen.fog_type = PVR_FOG_VERTEX;

	  /*cxt.blend.src = PVR_BLEND_ONE;
	  cxt.blend.dst = PVR_BLEND_ZERO;
	  cxt.txr.env = PVR_TXRENV_MODULATE;
	  cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
	  */
	  cxt.gen.specular = 1;
	  
	  pvr_poly_compile (&hdr, &cxt);

	  pvr_prim (&hdr, sizeof (hdr));
	  
	  //vert.fargb = 0;
	  vert.argb = PVR_PACK_COLOR (1.0f, 1.0f, 1.0f, 1.0f);
	  
	  //pvr_fog_far_depth (1);

	  mat_load (view->projection);

	  for (i = 0; i < str->length; i++)
	    {
	      int last = (i == str->length - 1);
	      float fogginess;
	      float *invec = &(*str->start)[i][0];
	      float x, y, z;

	      x = invec[0];
	      y = invec[1];
	      z = invec[2];
	      mat_trans_single (x, y, z);

	      vert.flags = (last) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
	      vert.argb = lightsource_diffuse (&(*str->start)[i][0],
					       &(*str->normals)[i][0],
					       &obj->ambient, &obj->pigment,
					       &lights->light0_pos_xform[0]);
	      fogginess
	        = perlin_noise_2D (x / 640.0 + amt,
				   y / 480.0 + amt / 2
				   + z / 2.0, 2) / 3.0;
	      if (fogginess < 0.0)
	        fogginess = 0.0;
	      if (fogginess > 1.0)
	        fogginess = 1.0;
	      fogginess += (1.0 - fogginess) * (z / 5.0);
	      	      
	      vert.oargb = PVR_PACK_COLOR (fogginess, 0.0, 0.0, 0.0);
	      vert.x = x;
	      vert.y = y;
	      vert.z = z;
	      vert.u = 0;
	      vert.v = 0;
	      pvr_prim (&vert, sizeof (vert));
	      if (i == 0 && str->inverse)
		pvr_prim (&vert, sizeof (vert));
	    }
	}

      /* Emit polygons for fake phong highlight.  */
      if (pass == 1 && obj->fake_phong)
        {
	  strip *strip_starts[1] = { 0 };
	  strip *strip_ends[1] = { 0 };
	  pvr_poly_cxt_t cxt;
	  pvr_poly_hdr_t hdr;
	  strip *str_out, *iter;
	  unsigned int intens = (255 << 24)
				| (obj->fake_phong->intensity << 16)
				| (obj->fake_phong->intensity << 8)
				| obj->fake_phong->intensity;

	  pvr_poly_cxt_txr (&cxt, ALPHA_LIST,
			    PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_TWIDDLED,
			    obj->fake_phong->xsize, obj->fake_phong->ysize,
			    obj->fake_phong->highlight, PVR_FILTER_BILINEAR);
	  	  
	  cxt.txr.uv_clamp = PVR_UVCLAMP_UV;
	  cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
	  cxt.blend.src = PVR_BLEND_ONE;
	  cxt.blend.dst = PVR_BLEND_ONE;
	  
	  pvr_poly_compile (&hdr, &cxt);
	  
	  /* Note: lightsource_fake_phong destroys xmtrx!  */
	  for (iter = use_strip; iter; iter = iter->next)
	    for (i = 0; i < iter->length; i++)
	      lightsource_fake_phong ((float *) &(*iter->start)[i][0],
				      (float *) &(*iter->normals)[i][0],
				      &lights->light0_pos_xform[0],
				      &lights->light0_up_xform[0],
				      &view->eye_pos[0],
				      view->inv_camera_orientation,
				      iter->v_attrs, i);

	  restrip_list (use_strip, fakephong_classify_triangle,
			strip_starts, strip_ends, &stripbuf, &capacity);

	  mat_load (view->projection);

	  for (str_out = strip_starts[0]; str_out; str_out = str_out->next)
            {
	      unsigned int i;
	      pvr_vertex_t vert;

              if (str_out->length < 3)
	        continue;

	      pvr_prim (&hdr, sizeof (hdr));

	      vert.argb = intens;
	      vert.oargb = 0;

	      for (i = 0; i < str_out->length; i++)
		{
		  float *invec = &(*str_out->start)[i][0];
		  float x, y, z;
		  int last = (i == str_out->length - 1);

		  //PREFETCH (&(*str_out->start)[i + 2][0]);

		  x = invec[0];
		  y = invec[1];
		  z = invec[2];
		  mat_trans_single (x, y, z);

		  //PREFETCH (&str_out->v_attrs[i + 1].fakephong.texc[0]);

		  vert.flags = (last) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
		  vert.x = x;
		  vert.y = y;
		  vert.z = z;
		  vert.u = str_out->v_attrs[i].fakephong.texc[0];
		  vert.v = str_out->v_attrs[i].fakephong.texc[1];
		  pvr_prim (&vert, sizeof (vert));
		  if (i == 0 && str_out->inverse)
		    pvr_prim (&vert, sizeof (vert));
		}
	    }
	}

      /* Bump mapping rendered as second pass.  It just modulates (darkens) the
         colour already in the buffer, so can't be used for bumpy specular
	 highlights -- the hardware's not capable of that.  */
      if (pass == 1 && obj->bump_map)
        {
	  float x_orient[3];
	  unsigned int i;
	  pvr_poly_cxt_t cxt;
	  pvr_poly_hdr_t hdr;
	  pvr_vertex_t vert;

	  pvr_poly_cxt_txr (&cxt, ALPHA_LIST,
			    PVR_TXRFMT_BUMP | PVR_TXRFMT_TWIDDLED,
			    use_strip->s_attrs->xsize,
			    use_strip->s_attrs->ysize,
			    use_strip->s_attrs->texture,
			    PVR_FILTER_BILINEAR);

	  cxt.blend.src = PVR_BLEND_ZERO;
	  cxt.blend.dst = PVR_BLEND_SRCALPHA;
	  cxt.txr.env = PVR_TXRENV_MODULATE;
	  cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
	  cxt.gen.specular = 1;

	  pvr_poly_compile (&hdr, &cxt);

	  /* I think this is wrong.  We probably want just the rotation part
	     of the modelview matrix, not the same inverse-transposed.
	     Otherwise maybe change the frame of reference and use one of the
	     other available matrices, or something.  */
	  vec_transform3_fipr (x_orient, (float *) obj_orient->normal_xform,
			       use_strip->s_attrs->uv_orient);
	  
	  pvr_prim (&hdr, sizeof (hdr));
	  
	  vert.argb = 0;  /* Not used for this blend mode.  */
	  
	  for (i = 0; i < use_strip->length; i++)
	    {
	      float light_incident[3], s_dot_n, t, f[3], d[3];
	      float d_cross_r[3], dxr_len, d_len, q;
	      int over_pi, over_2pi;
	      int last = (i == use_strip->length - 1);
	      float *invec = &(*str->start)[i][0];
	      float x, y, z;
	      
	      vec_sub (light_incident, &(*str->start)[i][0],
		       &lights->light0_pos[0]);
	      vec_normalize (light_incident, light_incident);
	      
	      s_dot_n = vec_dot (&(*str->normals)[i][0], light_incident);
	      /* Elevation (T) angle:
		 s.n = |s| |n| cos T
		 T = acos (s.n / (|s| * |n|))
		 |s| and |n| are both 1.  */
	      t = M_PI / 2 - acosf (s_dot_n);
	      
	      if (t < 0.0f)
	        t = 0.0f;

	      /* Rotation (Q) angle:
		 d x r = (|d| |r| sin Q) n
		 |d x r| / (|d| |r|) = sin Q.  */
	      vec_scale (f, &(*str->normals)[i][0], s_dot_n);
	      vec_sub (d, light_incident, f);
	      vec_cross (d_cross_r, d, x_orient);
	      dxr_len = vec_length (d_cross_r);
	      d_len = vec_length (d);
	      q = asinf (dxr_len / d_len);
	      
	      over_pi = vec_dot (d, x_orient) < 0.0f;
	      if (over_pi)
	        q = M_PI - q;
	
	      over_2pi = vec_dot (d_cross_r, &(*str->normals)[i][0]) < 0.0f;
	      if (over_2pi)
	        q = 2 * M_PI - q;

	      vert.oargb = bumpmap_set_bump_direction (t, 2 * M_PI - q,
			     obj->bump_map->intensity);

	      mat_load (view->projection);

	      x = invec[0];
	      y = invec[1];
	      z = invec[2];
	      mat_trans_single (x, y, z);

	      vert.flags = (last) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
	      vert.x = x;
	      vert.y = y;
	      vert.z = z;
	      vert.u = (*use_strip->texcoords)[i][0];
	      vert.v = (*use_strip->texcoords)[i][1];
	      pvr_prim (&vert, sizeof (vert));
	      if (i == 0 && use_strip->inverse)
	        pvr_prim (&vert, sizeof (vert));
	    }
	}
    }

  glKosMatrixDirty ();

  amt += 0.01;
}

strip *
strip_cons (strip *prev, unsigned int length, unsigned int alloc_bits)
{
  strip *newstrip = malloc (sizeof (strip));
  float (*geom)[][3] = NULL;
  float (*norms)[][3] = NULL;
  float (*texcoords)[][2] = NULL;
  vertex_attrs *v_attrs = NULL;
  
  if (alloc_bits & ALLOC_GEOMETRY)
    geom = malloc (3 * sizeof (float) * length);
  if (alloc_bits & ALLOC_NORMALS)
    norms = malloc (3 * sizeof (float) * length);
  if (alloc_bits & ALLOC_TEXCOORDS)
    texcoords = malloc (2 * sizeof (float) * length);
  if (alloc_bits & ALLOC_VERTEXATTRS)
    v_attrs = malloc (sizeof (vertex_attrs) * length);

  newstrip->start = geom;
  newstrip->normals = norms;
  newstrip->texcoords = texcoords;
  newstrip->length = length;
  newstrip->inverse = 0;
  newstrip->v_attrs = v_attrs;
  newstrip->s_attrs = NULL;
  newstrip->next = prev;
  
  return newstrip;
}

static strip *
strip_cons_pool (strip *prev, unsigned int length, unsigned int alloc_bits)
{
  strip *newstrip = pool_alloc (sizeof (strip));
  float (*geom)[][3] = NULL;
  float (*norms)[][3] = NULL;
  float (*texcoords)[][2] = NULL;
  vertex_attrs *v_attrs = NULL;
  
  if (alloc_bits & ALLOC_GEOMETRY)
    geom = pool_alloc (3 * sizeof (float) * length);
  if (alloc_bits & ALLOC_NORMALS)
    norms = pool_alloc (3 * sizeof (float) * length);
  if (alloc_bits & ALLOC_TEXCOORDS)
    texcoords = pool_alloc (2 * sizeof (float) * length);
  if (alloc_bits & ALLOC_VERTEXATTRS)
    v_attrs = pool_alloc (sizeof (vertex_attrs) * length);

  newstrip->start = geom;
  newstrip->normals = norms;
  newstrip->texcoords = texcoords;
  newstrip->length = length;
  newstrip->inverse = 0;
  newstrip->v_attrs = v_attrs;
  newstrip->s_attrs = NULL;
  newstrip->next = prev;
  
  return newstrip;
}
