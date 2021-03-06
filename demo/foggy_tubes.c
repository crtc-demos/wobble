#include <math.h>

#include <kos.h>
#include <png/png.h>
#include <kmg/kmg.h>

#include "vector.h"
#include "object.h"
#include "timing.h"
#include "tube.h"
#include "perlin.h"

#define ROWS 16
#define SEGMENTS 16

static object_orientation obj_orient;
static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));

static object *tube;
static vertexfog_info vert_fog;

static float amt = 0.0;

static float
fogging_fn (float x, float y, float z, vertex_attrs *v_attr)
{
  float fogginess = perlin_noise_2D (x / 160.0 + amt,
				     y / 120.0 + amt / 2
				     + z * 8.0, 2) / 3.0;

  if (fogginess < 0.0)
    fogginess = 0.0;
  if (fogginess > 1.0)
    fogginess = 1.0;

  fogginess += (1.0 - fogginess) * 0.02f / z;

  return fogginess;
}

static void
init_foggy_tube (void *params)
{
  tube = allocate_tube (ROWS, SEGMENTS);
  obj_orient.modelview = &mview;
  obj_orient.normal_xform = &normxform;
  object_set_clipping (tube, 1);
  object_set_ambient (tube, 32, 32, 32);
  object_set_pigment (tube, 224, 224, 224);
  
  vert_fog.fogging = &fogging_fn;
  
  //tube->vertex_fog = &vert_fog;
  tube->textured = 1;

  png_load_texture ("/rd/white.png", &vert_fog.texture, PNG_NO_ALPHA,
		    &vert_fog.w, &vert_fog.h);

  {
    kos_img_t tube_txr;
    pvr_ptr_t texaddr;
    
    kmg_to_img ("/rd/concrete_seamless.kmg", &tube_txr);
    texaddr = pvr_mem_malloc (tube_txr.byte_count);
    pvr_txr_load_kimg (&tube_txr, texaddr, 0);
    object_set_all_textures (tube, texaddr, tube_txr.w, tube_txr.h,
			     PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED
			     | PVR_TXRFMT_RGB565);
    kos_img_free (&tube_txr, 0);
  }

  pvr_set_bg_color (0.5, 0.5, 0.5);
}

static void
uninit_foggy_tube (void *params)
{
  pvr_set_bg_color (0, 0, 0);
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	       lighting *lights)
{
  fill_tube_data (tube, ROWS, SEGMENTS, (float) time_offset / 1000.0);
  glMatrixMode (GL_MODELVIEW);

  light_set_active (lights, 1);
  light_set_pos (lights, 0, 0.0, 2.0, -4.0);

  amt += 0.01;
}

static void
render_tubes (uint32_t time_offset, void *params, int iparam,
	       viewpoint *view, lighting *lights, int pass)
{
  int i;
  
  for (i = 0; i < 4; i++)
    {
      float amt = ((float) time_offset / 1000.0f) - 5;
      glKosMatrixDirty ();
      glPushMatrix ();
      glTranslatef (2.0 * (i - 1.5) - amt, 0, 4 + 2.0 * i - amt);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (view, tube, &obj_orient, lights, pass);
      glPopMatrix ();
    }
}

effect_methods foggy_tube_methods = {
  .preinit_assets = NULL,
  .init_effect = &init_foggy_tube,
  .prepare_frame = &prepare_frame,
  .display_effect = &render_tubes,
  .uninit_effect = &uninit_foggy_tube
};
