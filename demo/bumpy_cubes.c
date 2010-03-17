/* Bump-mapped cube.  Bits stolen from Fredrik Ehnbom.  */

#include <math.h>
#include <stdlib.h>

#include <kos.h>

#include "vector.h"
#include "object.h"
#include "timing.h"
#include "cube.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>

static object_orientation obj_orient0, obj_orient1, obj_orient2;
static matrix_t mview0 __attribute__((aligned(32)));
static matrix_t normxform0 __attribute__((aligned(32)));
static matrix_t mview1 __attribute__((aligned(32)));
static matrix_t normxform1 __attribute__((aligned(32)));
static matrix_t mview2 __attribute__((aligned(32)));
static matrix_t normxform2 __attribute__((aligned(32)));

static float rot1 = 0.0, rot2 = 0.0, rot3 = 0.0;
static object *cube;
bumpmap_info bumpinfo;

static void
preinit_bumpy_cubes (void)
{
  pvr_ptr_t bumptxr;
  int initialised = 0;
  
  if (initialised)
    return;

  cube = cube_create (1.0);

  bumptxr = bumpmap_load_raw ("/rd/bump.raw", 128, 128);
  object_set_all_textures (cube, bumptxr, 128, 128,
			   PVR_TXRFMT_BUMP | PVR_TXRFMT_TWIDDLED);
  bumpmap_auto_uv_orient (cube);
  
  object_set_ambient (cube, 0.2, 0.1, 0.0);
  object_set_pigment (cube, 1.0, 0.7, 0.0);
  
  bumpinfo.intensity = 1.0f;
  cube->bump_map = &bumpinfo;
  
  obj_orient0.modelview = &mview0;
  obj_orient0.normal_xform = &normxform0;
  obj_orient1.modelview = &mview1;
  obj_orient1.normal_xform = &normxform1;
  obj_orient2.modelview = &mview2;
  obj_orient2.normal_xform = &normxform2;
  
  initialised = 1;
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	       lighting *lights)
{
  float rot2_rad = rot2 * M_PI / 180.0f;
  float radius = 3.0f;

  light_set_pos (lights, 0, 0.0, 0.0, 4.0);

  glKosMatrixDirty ();

  glMatrixMode (GL_MODELVIEW);

  glPushMatrix ();
  glTranslatef (radius * fsin (rot2_rad), 0, 4 + radius * fcos (rot2_rad));
  glRotatef (rot2, 0.0, 1.0, 0.0);
  glRotatef (rot1, 1.0, 0.0, 0.0);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview0[0][0]);
  vec_normal_from_modelview (&normxform0[0][0], &mview0[0][0]);
  glPopMatrix ();

  glPushMatrix ();
  glTranslatef (radius * fsin (rot2_rad + 2.0 * M_PI / 3.0), 0,
		4 + radius * fcos (rot2_rad + 2.0 * M_PI / 3.0));
  glRotatef (rot2 + 120, 0.0, 1.0, 0.0);
  glRotatef (rot1 + 120, 1.0, 0.0, 0.0);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview1[0][0]);
  vec_normal_from_modelview (&normxform1[0][0], &mview1[0][0]);
  glPopMatrix ();

  glPushMatrix ();
  glTranslatef (radius * fsin (rot2_rad + 4.0 * M_PI / 3.0), 0,
		4 + radius * fcos (rot2_rad + 4.0 * M_PI / 3.0));
  glRotatef (rot2 + 240, 0.0, 1.0, 0.0);
  glRotatef (rot1 + 240, 1.0, 0.0, 0.0);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview2[0][0]);
  vec_normal_from_modelview (&normxform2[0][0], &mview2[0][0]);
  glPopMatrix ();

  rot1 += 1;
  rot2 += 2;
  rot3 += 0.3;
}

static void
render_bumpy_cubes (uint32_t time_offset, void *params, int iparam,
		    viewpoint *view, lighting *lights, int pass)
{
  light_set_active (lights, 1);
  object_render_immediate (view, cube, &obj_orient0, lights, pass);
  object_render_immediate (view, cube, &obj_orient1, lights, pass);
  object_render_immediate (view, cube, &obj_orient2, lights, pass);
}

effect_methods bumpy_cube_methods = {
  .preinit_assets = &preinit_bumpy_cubes,
  .init_effect = NULL,
  .prepare_frame = &prepare_frame,
  .display_effect = &render_bumpy_cubes,
  .uninit_effect = NULL
};
