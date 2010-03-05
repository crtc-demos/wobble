#include <stdint.h>

#include "object.h"
#include "torus.h"

#include "draw_torus.h"

object *torus_object;
static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));

static void
create_torus (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;

  torus_object = torus_create (1.0, 0.2, 16, 32);

  initialised = 1;
}

static void
draw_torus (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	    lighting *lights, int pass)
{
  torus_params *my_params = params;
  
  my_params->obj_orient.modelview = &mview;
  my_params->obj_orient.normal_xform = &normxform;

  glKosMatrixDirty ();
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  if (iparam == 1)
    glTranslatef (0, (float) time_offset / 60.0f, 4);
  else
    glTranslatef (0, 0, 4 + (float) time_offset / 120.0f);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  glPopMatrix ();

  object_set_ambient (torus_object, my_params->ambient_r, my_params->ambient_g,
		      my_params->ambient_b);
  object_set_pigment (torus_object, my_params->diffuse_r, my_params->diffuse_g,
		      my_params->diffuse_b);
  object_render_immediate (view, torus_object, &my_params->obj_orient, lights,
			   pass);
}

effect_methods torus_methods = {
  .preinit_assets = &create_torus,
  .init_effect = NULL,
  .display_effect = &draw_torus,
  .uninit_effect = NULL
};

