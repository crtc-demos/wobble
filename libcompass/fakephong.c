/* Fake phong highlighting.  */

#include <math.h>
#include <stdlib.h>
#include <alloca.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>

#include "vector.h"
#include "restrip.h"
#include "object.h"

/* Render a specular highlight of hardness HARDNESS into a (pre-allocated)
   texture HIGHLIGHT, of size XSIZE * YSIZE.  */

void
fakephong_highlight_texture (pvr_ptr_t highlight, unsigned int xsize,
			     unsigned int ysize, float hardness)
{
  uint8 *tmphilight;
  unsigned int x, y;
  float norm[3] = {0.0, 0.0, 1.0};
  const float xscale = (float) (xsize - 1) / 2.0f;
  const float yscale = (float) (ysize - 1) / 2.0f;
  
  tmphilight = malloc (xsize * ysize);
  
  /* We need a way to map 2D texture coordinates to points on a unit hemisphere,
     such that interpolating linearly along the texture approximates travelling
     smoothly along a great circle around the hemisphere.  Not sure how best to
     do this.  Maybe a parabolic texture?
     Using x, y coords unaltered and choosing z on the surface of the sphere. 
     Only the circular part of the texture is used.  */
  
  for (y = 0; y < ysize; y++)
    for (x = 0; x < xsize; x++)
      {
        float reflection[3];
	float dot;
	
	reflection[0] = (x - xscale) / xscale;
	reflection[1] = (y - yscale) / yscale;
	reflection[2] = fsqrt (1.0 - reflection[0] * reflection[0]
			       - reflection[1] * reflection[1]);
	
	dot = vec_dot (reflection, norm);
	
	tmphilight[y * ysize + x] = 255.0 * powf (dot, hardness);
      }
  
  if (highlight == 0)
    highlight = pvr_mem_malloc (xsize * ysize);

  pvr_txr_load_ex (tmphilight, highlight, xsize, ysize, PVR_TXRLOAD_8BPP);
  
  free (tmphilight);
}

/* LIGHT is the direction of the light from the vertex in question, and
   LIGHT_UPDIR is an "upwards" orientation vector for the light.  It doesn't
   have to be exactly perpendicular to the light vector, but it should be
   approximately so, as well as consistent for a given object.
   LIGHT, REFLECTION, LIGHT_UPDIR should be normalized before calling.  */

void
fakephong_texcoords (float *u, float *v, float reflection[3],
		     const float *light, const float *light_updir,
		     float *transformed_z)
{
  float light_sideways[3], light_realup[3];
  matrix_t light_basis;
  float reflection4[4], x_reflection[4];
  
  vec_cross (light_sideways, light, light_updir);
  vec_cross (light_realup, light_sideways, light);
  
  light_basis[0][0] = light_sideways[0];
  light_basis[1][0] = light_sideways[1];
  light_basis[2][0] = light_sideways[2];
  light_basis[3][0] = 0.0;
  
  light_basis[0][1] = light_realup[0];
  light_basis[1][1] = light_realup[1];
  light_basis[2][1] = light_realup[2];
  light_basis[3][1] = 0.0;
  
  light_basis[0][2] = light[0];
  light_basis[1][2] = light[1];
  light_basis[2][2] = light[2];
  light_basis[3][2] = 0.0;
  
  light_basis[0][3] = 0.0;
  light_basis[1][3] = 0.0;
  light_basis[2][3] = 0.0;
  light_basis[3][3] = 1.0;
  
  memcpy (reflection4, reflection, sizeof (float) * 3);
  reflection4[3] = 1.0;
  
  vec_transform (x_reflection, (float *) light_basis, reflection4);

  *transformed_z = x_reflection[2];

  *u = x_reflection[0] * 0.5 + 0.5;
  *v = x_reflection[1] * 0.5 + 0.5;
}

unsigned long
lightsource_diffuse (const float *vert, const float *norm,
		     const colour *ambient_col, const colour *pigment_col,
		     const float *light_pos)
{
  float light_to_vertex[3];
  float out;
  int r, g, b;

  vec_sub (light_to_vertex, vert, light_pos);
  vec_normalize (light_to_vertex, light_to_vertex);

  out = vec_dot (light_to_vertex, norm);

  if (out < 0)
    out = 0;

  r = ambient_col->r + out * (pigment_col->r - ambient_col->r);
  g = ambient_col->g + out * (pigment_col->g - ambient_col->g);
  b = ambient_col->b + out * (pigment_col->b - ambient_col->b);

  return (255 << 24) | (r << 16) | (g << 8) | b;
}


void
lightsource_fake_phong (float *vertex, float *norm, const float *light_pos,
			const float *light_up, const float *eye_pos,
			vertex_attrs *vertex_info, unsigned int idx)
{
  float norm_light[3];
  float eye_to_vertex[3], tmp[3], reflection[3];
  float eye_dot_norm;
  
  vec_normalize (norm_light, light_pos);
  
  vec_sub (eye_to_vertex, eye_pos, vertex);
  vec_normalize (eye_to_vertex, eye_to_vertex);
  
  eye_dot_norm = vec_dot (eye_to_vertex, norm);
  vec_scale (tmp, norm, 2.0 * eye_dot_norm);
  vec_sub (reflection, tmp, eye_to_vertex);
    
  fakephong_texcoords (&vertex_info[idx].fakephong.texc[0],
		       &vertex_info[idx].fakephong.texc[1],
		       reflection, norm_light, light_up,
		       &vertex_info[idx].fakephong.transformed_z);
}

#if 0
static strip *stripbuf = NULL;
static unsigned int capacity = 0;
#endif

int
fakephong_classify_triangle (float tri[3][3], int clockwise,
			     vertex_attrs *attrs)
{
#if 1
  if (attrs[0].fakephong.transformed_z >= 0
      || attrs[1].fakephong.transformed_z >= 0
      || attrs[2].fakephong.transformed_z >= 0)
    return 0;
  else
    return -1;
#else
  static int flipper = 0;
  flipper = -1 - flipper;
  return flipper;
  /*if ((rand () & 128) == 128)
    return 0;
  else
    return -1;*/
#endif
}

#if 0
void
render_geosphere (strip *strips, int pass)
{
  int sidx;
  strip *strip_starts[1] = { 0 };
  strip *strip_ends[1] = { 0 };
  strip *strip_iter, *to_draw = strips;
  
  if (capacity == 0)
    {
      capacity = 10;
      stripbuf = malloc (sizeof (strip) * capacity);
    }
      
  if (pass == 1)
    {
      glColor4ub (255, 255, 255, 255);

      for (strip_iter = strips; strip_iter; strip_iter = strip_iter->next)
	{
	  unsigned int strip_length = strip_iter->length;
	  float (*data)[][3] = strip_iter->start;
	  vertex_info *v_inf;
	  
	  v_inf = alloca (sizeof (vertex_info) * strip_length);

	  for (sidx = 0; sidx < strip_length; sidx++)
            {
	      float u, v;
	      lightsource_fake_phong (&u, &v, (float *) &(*data)[sidx],
				      (float *) &(*data)[sidx], pass,
				      &v_inf[sidx].transformed_z);

	      v_inf[sidx].texc[0] = u;
	      v_inf[sidx].texc[1] = v;
	    }

	  strip_iter->extra = v_inf;
	  strip_iter->extra_elsize = sizeof (vertex_info);
	}

      restrip_list (strips, classify_triangle, strip_starts, strip_ends,
		    &stripbuf, &capacity);
      to_draw = strip_starts[0];
    }

  for (strip_iter = to_draw; strip_iter; strip_iter = strip_iter->next)
    {
      int strip_length = strip_iter->length;
      float (*data)[][3] = strip_iter->start;
      vertex_info *v_inf = strip_iter->extra;
      int invert = strip_iter->inverse;

      glBegin (GL_TRIANGLE_STRIP);
      
      for (sidx = 0; sidx < strip_length; sidx++)
        {
	  if (pass == 0)
	    lightsource_fake_phong (0, 0, (float *) &(*data)[sidx],
				    (float *) &(*data)[sidx], pass, NULL);
          else
	    glTexCoord2f (v_inf[sidx].texc[0], v_inf[sidx].texc[1]);

	  if (invert == 1 && sidx == 0)
	    glVertex3fv ((GLfloat *) &(*data)[0]);

	  glVertex3fv ((GLfloat *) &(*data)[sidx]);
	}
      
      glEnd ();
    }
}
#endif
