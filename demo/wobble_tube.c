/* A wobbly tube.  */

#include <math.h>
#include <stdlib.h>
#include <alloca.h>
#include <stdint.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>
#include <kmg/kmg.h>

#define DC_FAST_MATHS 1
#include <dc/fmath.h>

#include "vector.h"
#include "restrip.h"
#include "fakephong.h"
#include "palette.h"
#include "object.h"
#include "loader.h"
#include "tube.h"
#include "skybox.h"
#include "timing.h"

#define ROWS 48
#define SEGMENTS 32

static float rot1 = 0;
static float rot2 = 0;
static float rot3 = 0;
static float rot4 = 0;
static float rot5 = 0;
static float rot6 = 0;
static float rot7 = 0;

static float eye_ang = 0;

static object *tube;
static object *skybox;
static pvr_ptr_t skytex[6];

static object_orientation obj_orient;

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));

static void
extra_wobbly_tube_fill (object *obj, int rows, int segments)
{
  int r, s;
  strip *strlist = obj->striplist;
  float points[ROWS + 1][SEGMENTS][3];
  float normals[ROWS + 1][SEGMENTS][3];
  
  for (r = 0; r < rows + 1; r++)
    {
      float ang = rot1 + 16 * (float) r / (float) rows;
      float mag = 1.0 + 0.35 * fsin (ang);

      for (s = 0; s < segments; s++)
        {
	  float sang = 2 * M_PI * (float) s / (float) segments;
	  float cosang = fcos (sang);
	  float sinang = fsin (sang);
	  
	  points[r][s][0] = mag * cosang;
	  points[r][s][1] = (16.0 * (float) r / (float) rows) - 8.0;
	  points[r][s][2] = mag * sinang;
	  
	  points[r][s][0] += fsin (points[r][s][1] + rot2)
			     + fsin (0.7 * points[r][s][1] + rot4);
			     + fsin (0.6 * points[r][s][1] + rot6);
	  points[r][s][2] += fsin (points[r][s][1] + rot3)
			     + fsin (0.7 * points[r][s][1] + rot5);
			     + fsin (0.6 * points[r][s][1] + rot7);
	}
    }
  
  for (r = 0; r < rows; r++)
    {
      for (s = 0; s < segments; s++)
        {
	  int next_s = (s == segments - 1) ? 0 : s + 1;
	  float rvec[3], svec[3];
	  
	  vec_sub (&rvec[0], &points[r + 1][s][0], &points[r][s][0]);
	  vec_sub (&svec[0], &points[r][next_s][0], &points[r][s][0]);
	  vec_cross (&normals[r][s][0], &svec[0], &rvec[0]);
	  vec_normalize (&normals[r][s][0], &normals[r][s][0]);
	}
    }
  
  for (r = 0; r < rows; r++)
    {
      float (*str)[][3];
      float (*norm)[][3];

      assert (strlist);
      
      str = strlist->start;
      norm = strlist->normals;
      strlist->inverse = 1;
      
      for (s = 0; s <= segments; s++)
        {
	  int use_s = (s == segments) ? 0 : s;

	  (*str)[s * 2 + 1][0] = points[r][use_s][0];
	  (*str)[s * 2 + 1][1] = points[r][use_s][1];
	  (*str)[s * 2 + 1][2] = points[r][use_s][2];

	  (*norm)[s * 2 + 1][0] = normals[r][use_s][0];
	  (*norm)[s * 2 + 1][1] = normals[r][use_s][1];
	  (*norm)[s * 2 + 1][2] = normals[r][use_s][2];

	  (*str)[s * 2][0] = points[r + 1][use_s][0];
	  (*str)[s * 2][1] = points[r + 1][use_s][1];
	  (*str)[s * 2][2] = points[r + 1][use_s][2];

	  (*norm)[s * 2][0] = normals[r + 1][use_s][0];
	  (*norm)[s * 2][1] = normals[r + 1][use_s][1];
	  (*norm)[s * 2][2] = normals[r + 1][use_s][2];
	}
      
      strlist = strlist->next;
    }
}

static void
preinit_tube (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;
  
  tube = allocate_tube (ROWS, SEGMENTS);

  object_set_ambient (tube, 64, 0, 0);
  object_set_pigment (tube, 255, 0, 0);
  object_set_clipping (tube, 0);

  printf ("Loading sky box texture");
  fflush (stdout);
  skytex[0] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky23.png", skytex[0], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[1] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky24.png", skytex[1], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[2] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky27.png", skytex[2], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[3] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky28.png", skytex[3], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[4] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky25.png", skytex[4], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[5] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky26.png", skytex[5], PNG_NO_ALPHA);
  putchar ('.');
  putchar ('\n');
  
  skybox = create_skybox (30, skytex, 512, 512);

  initialised = 1;
}

static void
render_wobble_tube (uint32_t time_offset, void *params, int iparam,
		    viewpoint *view, lighting *lights, int pass)
{
  obj_orient.modelview = &mview;
  obj_orient.normal_xform = &normxform;

  if (pass != 0)
    return;

  extra_wobbly_tube_fill (tube, ROWS, SEGMENTS);

  glMatrixMode (GL_MODELVIEW);

  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  object_render_immediate (view, skybox, &obj_orient, lights, 0);

  glKosMatrixDirty ();

  glPushMatrix ();
  glTranslatef (0, 0, 4);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  object_render_immediate (view, tube, &obj_orient, lights, 0);
  glPopMatrix ();

  rot1 += 0.05;
  rot2 += 0.04;
  rot3 += 0.03;
  rot4 += 0.045;
  rot5 += 0.035;
  rot6 += 0.06;
  rot7 += 0.055;
}

static void
uninit_wobble_tube (void *params)
{
  int i;
  
  for (i = 0; i < 6; i++)
    pvr_mem_free (skytex[i]);
}

effect_methods wobble_tube_methods = {
  .preinit_assets = &preinit_tube,
  .init_effect = NULL,
  .prepare_frame = NULL,
  .display_effect = &render_wobble_tube,
  .uninit_effect = &uninit_wobble_tube
};
