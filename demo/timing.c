/* Fake phong highlighting.  */

#include <math.h>
#include <stdlib.h>
#include <alloca.h>

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

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

float light_pos[] = {0, 0, -4.5};
float light_updir[] = {0.0, 1.0, 0.0};
float eye_pos[] = {0.0, 0.0, -4.5};

static matrix_t camera __attribute__((aligned(32)));
static matrix_t invcamera __attribute__((aligned(32)));
static matrix_t projection __attribute__((aligned(32)));
static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));

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

typedef struct {
  uint64_t start_time;
  uint64_t end_time;
  void (*do_thing) (uint64_t time_offset, void *params);
  void *params;
} do_thing_at;

static torus_params torus1 = {
  .colour = 0x000000ff;
};

static torus_params torus2 = {
  .colour = 0x0000ffff;
};

static do_thing_at sequence[] = {
  {    0, 1000, &draw_torus, &torus1 },
  { 1000, 2000, &draw_torus, &torus2 }
};

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  viewpoint view;
  object_orientation obj_orient;
  lighting lights;

  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_NTSC_IL, PM_RGB565);

  init_pvr ();

  object_set_ambient (tube, 64, 0, 0);
  object_set_pigment (tube, 255, 0, 0);
  object_set_clipping (tube, 1);
    
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
  
  obj_orient.modelview = &mview;
  obj_orient.normal_xform = &normxform;
  
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
  
  start_time = timer_ms_gettime64 ();
  
  while (!quit)
    {
      uint64_t current_time;

      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()

      current_time = timer_ms_gettime64 () - start_time;
      
      

      glMatrixMode (GL_MODELVIEW);
      glLoadIdentity ();
      gluLookAt (eye_pos[0], eye_pos[1], eye_pos[2],	/* Eye position.  */
		 0.0,  0.0,  0.0,			/* Centre.  */
		 0.0,  1.0,  0.0);			/* Up.  */

      glGetFloatv (GL_MODELVIEW_MATRIX, &camera[0][0]);
      vec_transpose_rotation (&invcamera[0][0], &camera[0][0]);
      memcpy (&view.eye_pos[0], &eye_pos[0], 3 * sizeof (float));
      vec_transform3_fipr (&lights.light0_pos_xform[0], &camera[0][0],
			   &lights.light0_pos[0]);
      vec_transform3_fipr (&lights.light0_up_xform[0], &camera[0][0],
			   &lights.light0_up[0]);

      glKosBeginFrame ();

      glKosMatrixDirty ();
      
      glKosFinishList ();

      glKosFinishFrame ();
    }

  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
