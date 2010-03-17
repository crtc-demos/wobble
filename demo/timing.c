/* Fake phong highlighting.  */

#include <math.h>
#include <stdlib.h>
#include <alloca.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
#include "fire.h"
#include "bumpy_cubes.h"
#include "foggy_tubes.h"

#undef PLAY_AUDIO
#define HOLD
#undef DEBUG

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

float light_pos[] = {0, 0, -4.5};
float light_updir[] = {0.0, 1.0, 0.0};
float eye_pos[] = {0.0, 0.0, -4.5};

#ifdef HOLD
static float facing[] = {0, 0, 1};
static float up_dir[] = {0.0, 1.0, 0.0};
#endif

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
    2 * 1024 * 1024,	/* Vertex buffer size 2M.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

uint64_t start_time;

/*
static torus_params torus1 = {
  .ambient_r = 64, .ambient_g = 0, .ambient_b = 0,
  .diffuse_r = 255, .diffuse_g = 0, .diffuse_b = 0
};

static torus_params torus2 = {
  .ambient_r = 64, .ambient_g = 64, .ambient_b = 0,
  .diffuse_r = 255, .diffuse_g = 255, .diffuse_b = 0
};
*/

static do_thing_at sequence[] = {
  /*{     0,  4000, &voronoi_methods, NULL, 0 },
  {  4000,  8000, &voronoi_methods, NULL, 1 },
  {  8000, 12000, &voronoi_methods, NULL, 2 },
  { 12000, 16000, &voronoi_methods, NULL, 3 },*/
  /*{ 20000, 22500, &torus_methods, &torus1, 0 },
  { 21000, 23500, &torus_methods, &torus2, 0 },
  { 22000, 24500, &torus_methods, &torus1, 1 },
  { 25000, 26000, &torus_methods, &torus2, 0 } */
  /*{     0, 300000, &wobble_tube_methods, NULL, 0 }*/
  { 0, 300000, &wave_methods, NULL, 0 }
  /*{     0,  10000, &wobble_tube_methods, NULL, 0 },*/
  /*{ 0, 600000, &fire_methods, NULL, 0 },
  { 0, 600000, &bumpy_cube_methods, NULL, 0 }*/
  /*{ 0, 22000, &foggy_tube_methods, NULL, 0 }*/
};

#define ARRAY_SIZE(X) (sizeof (X) / sizeof (X[0]))

#define MAX_ACTIVE 20

static unsigned char *audio_proxy;
static unsigned int proxy_length;
static unsigned int current_millis;

float audio_amplitude (void)
{
  unsigned int idx = current_millis / 10;
  
  if (idx < proxy_length)
    return (float) audio_proxy[idx] / 32.0f;
  else
    return 0;
}

#ifdef HOLD

void
rotate_around_y (float *vec, float ang)
{
  float ovec[3];
  float matrix[16] = {
    fcos (ang), 0, -fsin (ang), 0,
             0, 1,           0, 0,
    fsin (ang), 0,  fcos (ang), 0,
             0, 0,           0, 1
  };
  
  vec_transform3_fipr (&ovec[0], &matrix[0], vec);
  vec_normalize (vec, &ovec[0]);
}

void
rotate_around_x (float *vec, float ang)
{
  float ovec[3];
  float matrix[16] = {
    1,          0,           0, 0,
    0, fcos (ang), -fsin (ang), 0,
    0, fsin (ang),  fcos (ang), 0,
    0,          0,           0, 1
  };
  
  vec_transform3_fipr (&ovec[0], &matrix[0], vec);
  vec_normalize (vec, &ovec[0]);
}

#endif

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  viewpoint *view;
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
  
  view = view_allocate ();
  
  view_set_perspective (view,
			45.0,		/* Field of view in degrees.  */
			640.0 / 480.0,	/* Aspect ratio.  */
			1.0,		/* Z near.  */
			50.0);		/* Z far.  */

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

  view_set_eye_pos (view, eye_pos[0], eye_pos[1], eye_pos[2]);
  view_set_look_at (view, 0.0, 0.0, 0.0);
  view_set_up_dir (view, 0.0, 1.0, 0.0);

  light_set_pos (&lights, 0, light_pos[0], light_pos[1], light_pos[2]);
  light_set_up (&lights, 0, light_updir[0], light_updir[1], light_updir[2]);

  view_fix (view, &lights);
  light_fix (view, &lights);

  proxy_length = fs_load ("/rd/people_100hz.raw", (void **) &audio_proxy);
  printf ("Read %d bytes of audio proxy data\n", proxy_length);

#ifdef PLAY_AUDIO
  printf ("Starting CD audio...\n");
  spu_cdda_volume (15, 15);
  spu_cdda_pan (0, 31);
  cdrom_cdda_play (3, 3, 0, CDDA_TRACKS);
#endif

  start_time = timer_ms_gettime64 ();
  
  while (!quit)
    {
      uint64_t current_time;
      int i, j;

#ifdef HOLD
      {
        float angx = 0, angy = 0;
	float fwd = 0;
	float incr[3], horiz[3], vert[3];
	float pan_h = 0, pan_v = 0;
	float facing_uncam[3];

#ifdef DEBUG
        printf ("before controller loop\n");
#endif

	MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
          if (st->buttons & CONT_START)
	    quit = 1;
	  if (st->buttons & CONT_A)
	    eye_pos[0] += 0.05;
	  if (st->buttons & CONT_B)
	    eye_pos[0] -= 0.05;
	  if (st->buttons & CONT_DPAD_RIGHT)
	    pan_h = 0.05;
	  if (st->buttons & CONT_DPAD_LEFT)
	    pan_h = -0.05;
	  if (st->buttons & CONT_DPAD_UP)
	    pan_v = 0.05;
	  if (st->buttons & CONT_DPAD_DOWN)
	    pan_v = -0.05;
	  angx = st->joyx;
	  angy = st->joyy;
	  fwd = st->rtrig - st->ltrig;
	MAPLE_FOREACH_END ()

        vec_transform3_fipr (&facing_uncam[0],
			     &(*view->camera_orientation)[0][0],
			     &facing[0]);
	rotate_around_y (&facing_uncam[0], -angx / 5000.0f);
	rotate_around_x (&facing_uncam[0], -angy / 5000.0f);
	vec_transform3_fipr (&facing[0], &(*view->inv_camera_orientation)[0][0],
			     &facing_uncam[0]);
	fwd = fwd / 3000.0f;
	vec_scale (&incr[0], &facing[0], fwd);
	vec_add (&eye_pos[0], &eye_pos[0], &incr[0]);

	vec_cross (&incr[0], &facing[0], &up_dir[0]);
	vec_normalize (&incr[0], &incr[0]);
	vec_scale (&horiz[0], &incr[0], pan_h);
	vec_add (&eye_pos[0], &eye_pos[0], &horiz[0]);
	
	vec_cross (&vert[0], &incr[0], &facing[0]);
	vec_normalize (&vert[0], &vert[0]);
	vec_scale (&vert[0], &vert[0], pan_v);
	vec_add (&eye_pos[0], &eye_pos[0], &vert[0]);

	view_set_eye_pos (view, eye_pos[0], eye_pos[1], eye_pos[2]);
	view_set_look_at (view, eye_pos[0] + facing[0], eye_pos[1] + facing[1],
			  eye_pos[2] + facing[2]);

	glKosMatrixDirty ();
#ifdef DEBUG
	printf ("end of control loop\n");
#endif
      }

      current_time = timer_ms_gettime64 () - start_time; // 0;
#else
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()

      current_time = timer_ms_gettime64 () - start_time;
#endif

      /* For audio...  */
      current_millis = current_time;

#ifdef DEBUG
      printf ("terminate old effects\n");
#endif

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

#ifdef DEBUG
      printf ("start new effects\n");
#endif

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

      /* Do things we need to do before starting to send stuff to the PVR.  */

#ifdef DEBUG
      printf ("prepare frame (active effects=%d)\n", num_active_effects);
#endif
      
      for (i = 0; i < num_active_effects; i++)
        {
	  if (active_effects[i]->methods->prepare_frame)
	    {
#ifdef DEBUG
	      printf ("prepare frame: %p\n",
		      active_effects[i]->methods->prepare_frame);
#endif
	      active_effects[i]->methods->prepare_frame (
		current_time - active_effects[i]->start_time,
		active_effects[i]->params, active_effects[i]->iparam, view,
		&lights);
	    }
	}

#ifdef DEBUG
      printf ("fixing matrices\n");
#endif

      view_fix (view, &lights);
      light_fix (view, &lights);

#ifdef DEBUG
      printf ("begin frame\n");
#endif

      glKosBeginFrame ();

      for (i = 0; i < num_active_effects; i++)
        {
	  if (active_effects[i]->methods->display_effect)
	    active_effects[i]->methods->display_effect (
	      current_time - active_effects[i]->start_time,
	      active_effects[i]->params, active_effects[i]->iparam, view,
	      &lights, 0);
	}

      glKosFinishList ();
      glKosMatrixDirty ();

      for (i = 0; i < num_active_effects; i++)
        {
	  if (active_effects[i]->methods->display_effect)
	    active_effects[i]->methods->display_effect (
	      current_time - active_effects[i]->start_time,
	      active_effects[i]->params, active_effects[i]->iparam, view,
	      &lights, 1);
	}

      glKosFinishFrame ();
      
#ifdef DEBUG
      printf ("finished frame\n");
#endif
    }

  cdrom_spin_down ();

  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
