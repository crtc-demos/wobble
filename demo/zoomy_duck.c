#include <stdint.h>

#include "object.h"
#include "timing.h"
#include "loader.h"
#include "fakephong.h"

static object *duck;

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));
static object_orientation obj_orient = {
  .modelview = &mview,
  .normal_xform = &normxform,
  .dirty = 1
};

static pvr_ptr_t highlight;
static fakephong_info fphong;

static void
load_duck (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;
  
  duck = load_object ("/rd/duck8.strips");
  object_set_ambient (duck, 60, 60, 0);
  object_set_pigment (duck, 255, 255, 0);
  
  highlight = pvr_mem_malloc (128 * 128);
  fakephong_highlight_texture (highlight, 128, 128, 30.0);
  
  fphong.highlight = highlight;
  fphong.xsize = 128;
  fphong.ysize = 128;
  fphong.intensity = 255;
  
  duck->fake_phong = &fphong;
  
  initialised = 1;
}

static void
finalize_duck (void *params)
{
  pvr_mem_free (highlight);
}

static float rot1 = 0.0;

static void
prepare_frame (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	       lighting *lights)
{
  float zoom_amt = fsin (8 * M_PI * (float) time_offset / 1700.0);

  view_set_look_at (view, 0, 0, 0);
  view_set_eye_pos (view, 0, 0, 8);
  
  light_set_pos (lights, 0, 1, 1, 8);
  light_set_up (lights, 0, 0, 1, 0);

  glKosMatrixDirty ();
  glPushMatrix ();
  glTranslatef (0, 0, 2 + zoom_amt);
  glRotatef (rot1, 0, 1, 0);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  glPopMatrix ();
  
  rot1 += 1;
}

static void
render_duck (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	     lighting *lights, int pass)
{
  lights->active = 1;
  object_render_untextured_phong (view, duck, &obj_orient, lights, pass);
}

effect_methods zoomy_duck_methods = {
  .preinit_assets = &load_duck,
  .init_effect = NULL,
  .prepare_frame = &prepare_frame,
  .display_effect = &render_duck,
  .uninit_effect = NULL,
  .finalize = &finalize_duck
};
