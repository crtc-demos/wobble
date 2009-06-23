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
*/

#include <kos.h>

#include "restrip.h"
#include "object.h"

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
  newobj->fake_phong = NULL;
  newobj->env_map = NULL;
  newobj->bump_map = NULL;
  
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

static unsigned int
max_strip_length (const object *obj)
{
  unsigned int max_length = 0;
  strip *str;
  
  for (str = obj->striplist; str; str = str->next)
    if (str->length > max_length)
      max_length = str->length;

  return max_length;
}

static strip *stripbuf = NULL;
static unsigned int capacity = 0;

/* NORMAL_XFORM is the transpose-inverse of the rotation part of the MODELVIEW
   matrix.  */

void
object_render_immediate (const object *obj, int pass, matrix_t modelview,
			 matrix_t normal_xform, matrix_t projection,
			 matrix_t camera, matrix_t inv_camera_orientation,
			 const float *eye_pos, const float *light0_pos,
			 const float *light0_up)
{
  unsigned int strip_max = max_strip_length (obj), i;
  float (*trans)[][3], (*trans_norm)[][3];
  strip *str;
  
  if (capacity == 0)
    {
      capacity = 10;
      stripbuf = malloc (sizeof (strip) * capacity);
    }
  
  trans = malloc (sizeof (float) * 3 * strip_max);
  trans_norm = malloc (sizeof (float) * 3 * strip_max);
  
  for (str = obj->striplist; str; str = str->next)
    {
      /* This loop would benefit from assembly implementation.  */
      for (i = 0; i < str->length; i++)
        {
	  float vert[4], x_vert[4], norm[4], x_norm[4];
	  memcpy (vert, &(*str->start)[i][0], sizeof (float) * 3);
	  vert[3] = 1.0f;
	  memcpy (norm, &(*str->normals)[i][0], sizeof (float) * 3);
	  norm[3] = 1.0f;
	  vec_transform (x_vert, (float *) modelview, vert);
	  memcpy (&(*trans)[i][0], x_vert, sizeof (float) * 3);
	  vec_transform (x_norm, (float *) normal_xform, norm);
	  vec_normalize (&(*trans_norm)[i][0], x_norm);
	}

      if (pass == 0 && obj->fake_phong)
        {
	  /* First pass, dot-product lighting.  */
	  pvr_poly_cxt_t cxt;
	  pvr_poly_hdr_t hdr;
	  pvr_vertex_t vert;
	  
	  pvr_poly_cxt_col (&cxt, PVR_LIST_OP_POLY);
	  
	  pvr_poly_compile (&hdr, &cxt);

	  pvr_prim (&hdr, sizeof (hdr));
	  
	  vert.oargb = 0;
	  
	  for (i = 0; i < str->length; i++)
	    {
	      float vec[4], xvec[4];
	      int last = (i == str->length - 1);

	      memcpy (vec, &(*trans)[i][0], sizeof (float) * 3);
	      vec[3] = 1.0f;

	      vec_transform (&xvec[0], (float *) projection, &vec[0]);

	      vert.flags = (last) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
	      vert.argb = lightsource_diffuse (&(*trans)[i][0],
					       &(*trans_norm)[i][0],
					       &obj->ambient, &obj->pigment,
					       light0_pos);
	      vert.x = 320 + 320 * (xvec[0] / xvec[3]);
	      vert.y = 240 + 240 * (xvec[1] / xvec[3]);
	      vert.z = 1.0f / xvec[3];
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
	  strip transformed_strip, *str_out;

	  pvr_poly_cxt_txr (&cxt, PVR_LIST_PT_POLY,
			    PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_TWIDDLED,
			    obj->fake_phong->xsize, obj->fake_phong->ysize,
			    obj->fake_phong->highlight, PVR_FILTER_BILINEAR);
	  	  
	  cxt.txr.uv_clamp = PVR_UVCLAMP_UV;
	  cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
	  cxt.blend.src = PVR_BLEND_ONE;
	  cxt.blend.dst = PVR_BLEND_ONE;
	  
	  pvr_poly_compile (&hdr, &cxt);
	  
	  for (i = 0; i < str->length; i++)
	    lightsource_fake_phong ((float *) &(*trans)[i][0],
				    (float *) &(*trans_norm)[i][0], light0_pos,
				    light0_up, eye_pos, transformed_strip.attrs,
				    i);

	  transformed_strip.start = trans;
	  transformed_strip.length = str->length;
	  transformed_strip.normals = trans_norm;
	  transformed_strip.inverse = str->inverse;
	  transformed_strip.attrs = malloc (sizeof (vertex_attrs)
					    * str->length);
	  transformed_strip.next = NULL;

	  restrip_list (&transformed_strip, fakephong_classify_triangle,
			strip_starts, strip_ends, &stripbuf, &capacity);

	  for (str_out = strip_starts[0]; str_out; str_out = str_out->next)
            {
	      float xvec[4];
	      unsigned int i;
	      pvr_vertex_t vert;

              if (str_out->length < 3)
	        continue;

	      pvr_prim (&hdr, sizeof (hdr));

	      vert.argb = PVR_PACK_COLOR (1.0f, 1.0f, 1.0f, 1.0f);
	      vert.oargb = 0;

	      for (i = 0; i < str_out->length; i++)
		{
		  float vec[4];
		  int last = (i == str_out->length - 1);

		  memcpy (vec, &(*str_out->start)[i], sizeof (float) * 3);
		  vec[3] = 1.0f;

		  vec_transform (&xvec[0], (float *) projection, &vec[0]);

		  vert.flags = (last) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
		  vert.x = 320 + 320 * (xvec[0] / xvec[3]);
		  vert.y = 240 + 240 * (xvec[1] / xvec[3]);
		  vert.z = 1.0f / xvec[3];
		  vert.u = str_out->attrs[i].fakephong.texc[0];
		  vert.v = str_out->attrs[i].fakephong.texc[1];
		  pvr_prim (&vert, sizeof (vert));
		  if (i == 0 && str_out->inverse)
		    pvr_prim (&vert, sizeof (vert));
		}
	    }
	  free (transformed_strip.attrs);
	}
    }
  
  free (trans);
  free (trans_norm);
}
