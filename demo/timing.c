/* Fake phong highlighting.  */

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

#include "draw_torus.h"
#include "voronoi.h"
#include "wobble_tube.h"
#include "waves.h"

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

float light_pos[] = {0, 0, -4.5};
float light_updir[] = {0.0, 1.0, 0.0};
float eye_pos[] = {0.0, 0.0, -4.5};

static matrix_t camera __attribute__((aligned(32)));
static matrix_t invcamera __attribute__((aligned(32)));
static matrix_t projection __attribute__((aligned(32)));

static matrix_t screen_mat __attribute__((aligned(32))) =
  {
    { 320.0,  0,     0, 0 },
    { 0,     -240.0, 0, 0 },
    { 0,      0,     1, 0 },
    { 320.0,  240.0, 0, 1 }
  };

pvr_ptr_t highlight = 0;

static void
init_pvr (void)
{
  pvr_init_params_t params = {
    { PVR_BINSIZE_32,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_32,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_0 },	/* Punch-thrus.  */
    512 * 1024,		/* Vertex buffer size 512K.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

uint64_t start_time;

static torus_params torus1 = {
  .ambient_r = 64, .ambient_g = 0, .ambient_b = 0,
  .diffuse_r = 255, .diffuse_g = 0, .diffuse_b = 0
};

static torus_params torus2 = {
  .ambient_r = 64, .ambient_g = 64, .ambient_b = 0,
  .diffuse_r = 255, .diffuse_g = 255, .diffuse_b = 0
};

static do_thing_at sequence[] = {
 /* {     0,  4000, &voronoi_methods, NULL, 0 },
  {  4000,  8000, &voronoi_methods, NULL, 1 },
  {  8000, 12000, &voronoi_methods, NULL, 2 },
  { 12000, 16000, &voronoi_methods, NULL, 3 },
  { 20000, 22500, &torus_methods, &torus1, 0 },
  { 21000, 23500, &torus_methods, &torus2, 0 },
  { 22000, 24500, &torus_methods, &torus1, 1 },
  { 25000, 26000, &torus_methods, &torus2, 0 } */
  /*{     0, 30000, &wobble_tube_methods, NULL, 0 }*/
  { 0, 300000, &wave_methods, NULL, 0 }
};

#define ARRAY_SIZE(X) (sizeof (X) / sizeof (X[0]))

#define MAX_ACTIVE 20

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  viewpoint view;
  lighting lights;
  uint64_t start_time;
  int i;
  unsigned int next_effect;
  do_thing_at *active_effects[MAX_ACTIVE];
  unsigned int num_active_effects;
  const int num_effects = ARRAY_SIZE (sequence);

  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_NTSC_IL, PM_RGB565);

  init_pvr ();

  glKosInit ();
  
  glViewport (0, 0, 640, 480);
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glShadeModel (GL_SMOOTH);
  glClearDepth (1.0f);
  glDepthFunc (GL_LEQUAL);
  
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (45.0,			/* Field of view in degrees.  */
		  640.0 / 480.0,	/* Aspect ratio.  */
		  1.0,			/* Z near.  */
		  50.0);		/* Z far.  */

  glGetFloatv (GL_PROJECTION_MATRIX, &projection[0][0]);

  mat_load (&screen_mat);
  mat_apply (&projection);
  mat_store (&projection);
  glKosMatrixDirty ();

  view.projection = &projection;
  view.camera = &camera;
  view.inv_camera_orientation = &invcamera;
  view.near = -0.2f;
    
  memcpy (&lights.light0_pos[0], &light_pos[0], 3 * sizeof (float));
  memcpy (&lights.light0_up[0], &light_updir[0], 3 * sizeof (float));
  vec_transform3_fipr (&lights.light0_pos_xform[0], &camera[0][0],
		       &lights.light0_pos[0]);
  vec_transform3_fipr (&lights.light0_up_xform[0], &camera[0][0],
		       &lights.light0_up[0]);

  /* glGenTextures (1, &texture[0]); */

  palette_grey_ramp ();

  glBlendFunc (GL_ONE, GL_ONE);
  
  pvr_set_bg_color (0.0, 0.0, 0.0);
  
  for (i = 0; i < ARRAY_SIZE (sequence); i++)
    {
      if (sequence[i].methods->preinit_assets)
        sequence[i].methods->preinit_assets ();
    }
  
  num_active_effects = 0;
  next_effect = 0;

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (eye_pos[0], eye_pos[1], eye_pos[2],	/* Eye position.  */
	     0.0,  0.0,  0.0,				/* Centre.  */
	     0.0,  1.0,  0.0);				/* Up.  */

  glGetFloatv (GL_MODELVIEW_MATRIX, &camera[0][0]);
  vec_transpose_rotation (&invcamera[0][0], &camera[0][0]);
  memcpy (&view.eye_pos[0], &eye_pos[0], 3 * sizeof (float));
  vec_transform3_fipr (&lights.light0_pos_xform[0], &camera[0][0],
		       &lights.light0_pos[0]);
  vec_transform3_fipr (&lights.light0_up_xform[0], &camera[0][0],
		       &lights.light0_up[0]);

  start_time = timer_ms_gettime64 ();
  
  while (!quit)
    {
      uint64_t current_time;
      int i, j;

      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()

      current_time = timer_ms_gettime64 () - start_time;
      
      glKosBeginFrame ();

      /* Terminate old effects.  */
      for (i = 0; i < num_active_effects; i++)
        {
	  if (current_time >= active_effects[i]->end_time)
	    {
	      /*printf ("uninit effect %d (iparam=%d)\n", i,
		      active_effects[i]->iparam);*/

	      if (active_effects[i]->methods->uninit_effect)
	        {
	          active_effects[i]->methods->uninit_effect (
		    active_effects[i]->params);
		}
	      active_effects[i] = NULL;
	    }
	}

      /* And remove from active list.  */
      for (i = 0, j = 0; j < num_active_effects;)
        {
	  active_effects[i] = active_effects[j];

	  if (active_effects[i] == NULL)
	    j++;
	  else
	    {
	      i++;
	      j++;
	    }
	}

      num_active_effects = i;

      while (next_effect < num_effects
	     && current_time >= sequence[next_effect].start_time)
	{
	  active_effects[num_active_effects] = &sequence[next_effect];

	  /*printf ("init effect %d (%p, iparam=%d)\n", next_effect,
		  sequence[next_effect].methods->init_effect,
		  sequence[next_effect].iparam);*/

	  if (sequence[next_effect].methods->init_effect)
	    {
	      sequence[next_effect].methods->init_effect (
		sequence[next_effect].params);
	    }

	  num_active_effects++;
	  next_effect++;
	}

      if (next_effect == num_effects && num_active_effects == 0)
        quit = 1;

      for (i = 0; i < num_active_effects; i++)
        {
	  if (active_effects[i]->methods->display_effect)
	    active_effects[i]->methods->display_effect (
	      current_time - active_effects[i]->start_time,
	      active_effects[i]->params, active_effects[i]->iparam, &view,
	      &lights, 0);
	}

      glKosFinishList ();
      glKosMatrixDirty ();

      for (i = 0; i < num_active_effects; i++)
        {
	  if (active_effects[i]->methods->display_effect)
	    active_effects[i]->methods->display_effect (
	      current_time - active_effects[i]->start_time,
	      active_effects[i]->params, active_effects[i]->iparam, &view,
	      &lights, 1);
	}

      glKosFinishFrame ();
    }

  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
