/* Shiny thing.  */

#include <stdint.h>

#include <kos.h>
#include <png/png.h>

#include "timing.h"
#include "object.h"
#include "loader.h"

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));
static object_orientation obj_orient = {
  .modelview = &mview,
  .normal_xform = &normxform,
  .dirty = 1
};

static object *thing;
static envmap_dual_para_info envmap;

void
preinit_effect (void)
{
  thing = load_object ("/rd/potato.strips");
  
  envmap.front_txr = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky21.png", envmap.front_txr, PNG_NO_ALPHA);
  envmap.front_txr_fmt = PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED;
  
  envmap.back_txr = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky22o.png", envmap.back_txr, PNG_MASK_ALPHA);
  envmap.back_txr_fmt = PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_TWIDDLED;
  
  envmap.xsize = 512;
  envmap.ysize = 512;
  
  thing->env_map = &envmap;
}

void
free_textures (void *params)
{
  pvr_mem_free (envmap.back_txr);
  pvr_mem_free (envmap.front_txr);
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam,
	       viewpoint *view, lighting *lights)
{
  /* FIXME: Use a nice camera path instead!  */
  view_set_eye_pos (view, 0, 4, 15);
  view_set_look_at (view, 0, 0, 0);
}

static void
display_shiny_thing (uint32_t time_offset, void *params, int iparam,
		     viewpoint *view, lighting *lights, int pass)
{
  float rot = (float) time_offset / 10.0;
  float amp = audio_amplitude () / 4.0;

  if (pass != PVR_LIST_OP_POLY && pass != PVR_LIST_PT_POLY)
    return;

  glPushMatrix ();
  glRotatef (rot, 0, 1, 0);
  glScalef (1.0 + amp, 1.0 + amp, 1.0 + amp);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  glPopMatrix ();

  object_render_immediate (view, thing, &obj_orient, lights, pass);
}

effect_methods shiny_thing_methods = {
  .preinit_assets = &preinit_effect,
  .init_effect = NULL,
  .prepare_frame = &prepare_frame,
  .display_effect = &display_shiny_thing,
  .uninit_effect = NULL,
  .finalize = &free_textures
};
