/* Duck fountain!  */

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
#include "timing.h"

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));
static object_orientation obj_orient = {
  .modelview = &mview,
  .normal_xform = &normxform,
  .dirty = 1
};

#define DUCKS 10

static float rot1 = 0;

static float pos[DUCKS][3];
static float rotaxis[DUCKS][3];
static float velocity[DUCKS][3];
static char duck_colour[DUCKS];
static int active = 0;
static int spawnctr = 0;
static object *duck;
static object *shooter;

static void
preinit_effect (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;

  duck = load_object ("/rd/duck_hi.strips");

  object_set_ambient (duck, 64, 0, 0);
  object_set_pigment (duck, 255, 0, 0);
  object_set_clipping (duck, 0);
  
  shooter = load_object ("/rd/duck_shooter.strips");
    
  initialised = 1;
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	       lighting *lights)
{
  unsigned int i;

  view_set_eye_pos (view, 0, 0, 15);
  view_set_look_at (view, 0, 0, 0);

  for (i = 0; i < active; i++)
    {
      pos[i][0] += velocity[i][0];
      pos[i][1] += velocity[i][1];
      pos[i][2] += velocity[i][2];

      velocity[i][1] -= 0.01;
      if (pos[i][1] < -6)
	{
	  pos[i][0] = 0;
	  pos[i][1] = -2;
	  pos[i][2] = -4;
	  velocity[i][0] = ((float) (rand() & 255) - 127.5) / 500.0;
	  velocity[i][1] = 0.3;
	  velocity[i][2] = ((float) (rand() & 255) - 127.5) / 500.0;
	}
    }

  if (spawnctr > 6 && active < DUCKS)
    {
      pos[active][0] = 0;
      pos[active][1] = -2;
      pos[active][2] = -4;
      velocity[active][0] = ((float) (rand() & 255) - 127.5) / 500.0;
      velocity[active][1] = 0.3;
      velocity[active][2] = ((float) (rand() & 255) - 127.5) / 500.0;
      rotaxis[active][0] = ((float) (rand() & 255) - 127.5) / 127.5;
      rotaxis[active][1] = ((float) (rand() & 255) - 127.5) / 127.5;
      rotaxis[active][2] = ((float) (rand() & 255) - 127.5) / 127.5;
      duck_colour[active] = rand() & 1;
      vec_normalize (&rotaxis[active][0], &rotaxis[active][0]);
      active++;
      spawnctr = 0;
    }

  spawnctr += 1;
  rot1 += 2;
}

void
display_fountain (uint32_t time_offset, void *params, int iparam,
		  viewpoint *view, lighting *lights, int pass)
{
  int i;
  
  if (pass != PVR_LIST_OP_POLY)
    return;

  lights->active = 1;
  
  for (i = 0; i < active; i++)
    {
      if (duck_colour[i])
	{
	  object_set_ambient (duck, 64, 64, 0);
	  object_set_pigment (duck, 255, 255, 0);
	}
      else
	{
	  object_set_ambient (duck, 64, 32, 64);
	  object_set_pigment (duck, 255, 127, 255);
	}

      glPushMatrix ();
      glTranslatef (pos[i][0], pos[i][1], pos[i][2]);
      glRotatef (rot1, rotaxis[i][0], rotaxis[i][1], rotaxis[i][2]);
      glScalef (0.3, 0.3, 0.3);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (view, duck, &obj_orient, lights, pass);
      glPopMatrix ();
    }

  glPushMatrix ();
  glTranslatef (0, -2, -4);  /* The spawn position.  */
  glScalef (2, 2, 2);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  object_render_immediate (view, shooter, &obj_orient, lights, pass);
  glPopMatrix ();
}

effect_methods duck_fountain_methods = {
  .preinit_assets = &preinit_effect,
  .init_effect = NULL,
  .prepare_frame = &prepare_frame,
  .display_effect = &display_fountain,
  .uninit_effect = NULL,
  .finalize = NULL
};
