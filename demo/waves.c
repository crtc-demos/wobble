#include <math.h>
#include <stdint.h>

#include <kos.h>
#include <kmg/kmg.h>
#include <png/png.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vector.h"
#include "object.h"
#include "palette.h"
#include "timing.h"

#define XSIZE 64
#define YSIZE 64

/*** we want to render water as triangles:
      ____________..
     /\  /\  /\  /\
    /__\/__\/__\/__\..
   /\  /\  /\  /\  /\
  /__\/__\/__\/__\/__\..

Realign this to a grid like so:
  _____________
  |\  |\  |\  |
  |__\|__\|__\|
  |\  |\  |\  |
  |__\|__\|__\|

so a point like this:

    1_____2
    /\   /\
   /  \ /  \
  6----*----3
   \  / \  /
    \/___\/
    5     4

has equidistant neighbours like this:

   1---2
   |   |
   6---*---3
       |   |
       5---4

***/

#define HSCALE	3.0
#define VSCALE	0.25

static float pos[YSIZE][XSIZE];
static float vel[YSIZE][XSIZE];
static float normals[YSIZE][XSIZE][3];

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));

static void
init_grid (void)
{
  unsigned int i, j;
  
  for (j = 0; j < YSIZE; j++)
    for (i = 0; i < XSIZE; i++)
      pos[j][i] = vel[j][i] = 0.0;
}

static float rot1 = 0.0;
static float rot2 = 0.0;

static void
update_grid (void)
{
  const float timestep = 0.05;
#if 0
  const float spring = 0.17;
  const float mass = 0.04;
  const float drag = 0.0045;
#else
  const float spring = 0.17;
  const float mass = 0.02;
  const float drag = 0.0045;
#endif
  int i, j;
  float amp = audio_amplitude ();

  const float yfac = sin (M_PI / 3.0);
  const float iscale = 1.0 / (float) XSIZE, jscale = yfac / (float) YSIZE;
  const float ihalf = iscale / 2.0;

  pos[19][15] = fsin (rot1) / 2;
  pos[33][47] = fsin (rot2) / 3;
  
  rot1 += 0.1 * amp;
  if (rot1 > 2 * M_PI)
    rot1 -= 2 * M_PI;

  rot2 += 0.05 * amp;
  if (rot2 > 2 * M_PI)
    rot2 -= 2 * M_PI;

  for (j=1; j<YSIZE-1; j++)
  {
    for (i=1; i<XSIZE-1; i++)
    {
      float f, a, k1, k2;
      float restpos = (pos[j+1][i-1] +
                       pos[j+1][i] +
                       pos[j][i+1] +
                       pos[j-1][i+1] +
                       pos[j-1][i] +
                       pos[j][i-1]) * (1.0/6.0);

      f = (restpos - pos[j][i])*spring - drag*vel[j][i];
      a = f * (1.0 / mass);
      k1 = timestep * a;
      
      f = (restpos - pos[j][i])*spring - drag*(vel[j][i]+k1);
      a = f * (1.0 / mass);
      k2 = timestep * a;

      vel[j][i] = vel[j][i] + (k1+k2) * 0.5;
      pos[j][i] = pos[j][i] + vel[j][i]*timestep;
    }
  }
  
  /* copy boundary */
  for (i=0; i<XSIZE; i++)
  {
    pos[0][i] = pos[1][i];
    pos[YSIZE-1][i] = pos[YSIZE-2][i];
  }
  for (j=0; j<YSIZE; j++)
  {
    pos[j][0] = pos[j][1];
    pos[j][XSIZE-1] = pos[j][XSIZE-2];
  }
  
  /* Calculate normals.  */
  for (j = 1; j < YSIZE - 1; j++)
    for (i = 1; i < XSIZE - 1; i++)
      {
	float p1, p2, p3, p4, p5, p6;
	float xgrad, ygrad;
	float centre = pos[j][i];
	float ivec[3], jvec[3], norm[3];
	
	/***
	
	  +         <-+
	  |           | VSCALE * diff
	  |       + <-+
	  |       |
	  +-------+
	  
	  ^-------^
	HSCALE * iscale

	gradient = (VSCALE * diff) / (HSCALE * iscale)
		 = diff * (VSCALE / (HSCALE * iscale))

	***/
	
	p2 = pos[j + 1][i] - centre;		/* 2 */
	p3 = pos[j][i + 1] - centre;		/* 3 */
	p4 = pos[j - 1][i + 1] - centre;	/* 4 */
	xgrad = fipr (p2, p3, p4, 0.0f,
		      VSCALE / (HSCALE * ihalf),
		      VSCALE / (HSCALE * iscale),
		      VSCALE / (HSCALE * ihalf), 0.0f);
	
	p1 = pos[j + 1][i - 1] - centre;	/* 1 */
	p6 = pos[j][i - 1] - centre;		/* 6 */
	p5 = pos[j - 1][i] - centre;		/* 5 */
	xgrad += fipr (p1, p6, p5, 0.0f,
		       VSCALE / (HSCALE * -ihalf),
		       VSCALE / (HSCALE * -iscale),
		       VSCALE / (HSCALE * -ihalf), 0.0f);
	
	ygrad = fipr (p1, p2, p4, p5,
		      VSCALE / (HSCALE * jscale),
		      VSCALE / (HSCALE * jscale),
		      VSCALE / (HSCALE * -jscale),
		      VSCALE / (HSCALE * -jscale));
	
	ivec[0] = 1.0f;
	ivec[1] = xgrad;
	ivec[2] = 0.0f;
	
	jvec[0] = 0.0f;
	jvec[1] = ygrad;
	jvec[2] = 1.0f;
	
	vec_cross (&norm[0], &jvec[0], &ivec[0]);
	vec_normalize (&normals[j][i][0], &norm[0]);
      }
}

static object *
allocate_water (unsigned int xsize, unsigned int ysize)
{
  strip *previous = NULL;
  int y;

  for (y = 1; y < ysize - 2; y++)
    {
      strip *newstrip = strip_cons (previous, (xsize - 3) * 2,
				    ALLOC_GEOMETRY | ALLOC_NORMALS);
      previous = newstrip;
    }

  return object_create_default (previous);
}

static void
update_strips (object *obj, int clockwise)
{
  unsigned int i, j;
  const float yfac = sin (M_PI / 3.0);
  const float iscale = 1.0 / (float) XSIZE, jscale = yfac / (float) YSIZE;
  const float ihalf = iscale / 2.0;
  strip *mystrip = obj->striplist;

  for (j = 1; j < YSIZE - 2; j++)
    {
      float js = (float) j * jscale;
      float (*geom)[][3] = mystrip->start;
      float (*norm)[][3] = mystrip->normals;
      
      mystrip->inverse = 1;
      
      assert (mystrip);
      assert (geom);
      assert (norm);
      
      for (i = 1; i < XSIZE - 2; i++)
        {
	  float is = (float) i * iscale + (float) j * ihalf;
	  int idx = (i - 1) * 2;

	  (*geom)[idx][0] = is * HSCALE;
	  (*geom)[idx][1] = pos[j][i] * VSCALE;
	  (*geom)[idx][2] = js * HSCALE;
	  
	  (*norm)[idx][0] = normals[j][i][0];
	  (*norm)[idx][1] = normals[j][i][1];
	  (*norm)[idx][2] = normals[j][i][2];

	  (*geom)[idx + 1][0] = (is + ihalf) * HSCALE;
	  (*geom)[idx + 1][1] = pos[j + 1][i] * VSCALE;
	  (*geom)[idx + 1][2] = (js + jscale) * HSCALE;

	  (*norm)[idx + 1][0] = normals[j + 1][i][0];
	  (*norm)[idx + 1][1] = normals[j + 1][i][1];
	  (*norm)[idx + 1][2] = normals[j + 1][i][2];
	}

      mystrip = mystrip->next;
    }
}

static pvr_ptr_t highlight = 0;
static object *water;
static fakephong_info f_phong;
static envmap_dual_para_info envmap;
static object_orientation obj_orient;

static void
preinit_water (void)
{
  water = allocate_water (XSIZE, YSIZE);

#if 1
  highlight = pvr_mem_malloc (256 * 256);
  fakephong_highlight_texture (highlight, 256, 256, 50.0f);

  f_phong.highlight = highlight;
  f_phong.xsize = 256;
  f_phong.ysize = 256;
  f_phong.intensity = 255;
  
  water->fake_phong = &f_phong;
#elif 0
/*
  kmg_to_img ("/rd/sky21.kmg", &front_txr);
  texaddr = pvr_mem_malloc (front_txr.byte_count);
  pvr_txr_load_kimg (&front_txr, texaddr, PVR_TXRFMT_VQ_ENABLE);
  envmap.front_txr = texaddr;
  kos_img_free (&front_txr, 0);
  
  kmg_to_img ("/rd/sky22.kmg", &back_txr);
  texaddr = pvr_mem_malloc (back_txr.byte_count);
  pvr_txr_load_kimg (&back_txr, texaddr, PVR_TXRFMT_VQ_ENABLE);
  envmap.back_txr = texaddr;
  kos_img_free (&back_txr, 0);

  envmap.xsize = front_txr.w;
  envmap.ysize = front_txr.h;
*/
  envmap.front_txr = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky21.png", envmap.front_txr, PNG_NO_ALPHA);
  envmap.front_txr_fmt = PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED;

  envmap.back_txr = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky22o.png", envmap.back_txr, PNG_MASK_ALPHA);
  envmap.back_txr_fmt = PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_TWIDDLED;
  
  envmap.xsize = 512;
  envmap.ysize = 512;
  
  water->env_map = &envmap;
#endif

  object_set_ambient (water, 0, 0, 64);
  object_set_pigment (water, 0, 0, 255);
  object_set_clipping (water, 0);
  
  obj_orient.modelview = &mview;
  obj_orient.normal_xform = &normxform;
}

static void
init_water (void *params)
{
  init_grid ();
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	       lighting *lights)
{
  update_grid ();
  update_grid ();
  update_strips (water, 1);

  light_set_active (lights, 1);
  light_set_pos (lights, 0, 0, 5, 0);

  glKosMatrixDirty ();

  /*view_set_look_at (view, 0.866, 0.5, 0.5);
  view_set_up_dir (view, 0.0, 1.0, 0.0);*/
}

static void
render_water (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	      lighting *lights, int pass)
{
  glPushMatrix ();
  glTranslatef (0, -2, 0);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  object_render_immediate (view, water, &obj_orient, lights, pass);
  glPopMatrix ();
}

effect_methods wave_methods = {
  .preinit_assets = &preinit_water,
  .init_effect = &init_water,
  .prepare_frame = &prepare_frame,
  .display_effect = &render_water,
  .uninit_effect = NULL
};
