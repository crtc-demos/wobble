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
#include "tube.h"

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
    { PVR_BINSIZE_16,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_16,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_0 },	/* Punch-thrus.  */
    512 * 1024,		/* Vertex buffer size 512K.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}


#define ROWS 16
#define SEGMENTS 32

float rot1 = 0;
float eye_ang = 0;

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  object *tube = allocate_tube (ROWS, SEGMENTS);
  object *skybox;
  fakephong_info f_phong;
  viewpoint view;
  object_orientation obj_orient;
  lighting lights;
  pvr_ptr_t skytex[6];

  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_NTSC_IL, PM_RGB565);

  init_pvr ();

  highlight = pvr_mem_malloc (256 * 256);
  fakephong_highlight_texture (highlight, 256, 256, 10.0f);

  //tube = load_object ("/rd/out.stp");

  f_phong.highlight = highlight;
  f_phong.xsize = 256;
  f_phong.ysize = 256;
  f_phong.intensity = 128;

  tube->fake_phong = &f_phong;

  object_set_ambient (tube, 64, 0, 0);
  object_set_pigment (tube, 255, 0, 0);
  object_set_clipping (tube, 1);
  
  skytex[0] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky23.png", skytex[0], PNG_NO_ALPHA);
  skytex[1] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky24.png", skytex[1], PNG_NO_ALPHA);
  skytex[2] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky27.png", skytex[2], PNG_NO_ALPHA);
  skytex[3] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky28.png", skytex[3], PNG_NO_ALPHA);
  skytex[4] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky25.png", skytex[4], PNG_NO_ALPHA);
  skytex[5] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky26.png", skytex[5], PNG_NO_ALPHA);
  
  skybox = create_skybox (30, skytex, 512, 512);
  
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
  
  while (!quit)
    {
      float pushx = 0.0, pushy = 0.0;

      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
	pushx = st->joyx / 25.0;
	pushy = (st->joyy / 25.0) + 2.0;
      MAPLE_FOREACH_END ()

      fill_tube_data (tube, ROWS, SEGMENTS, rot1);

      eye_pos[0] = 4.5 * sinf (eye_ang);
      eye_pos[2] = 4.5 * cosf (eye_ang);
      eye_pos[1] = 2.0 - pushy;
      
      eye_ang -= pushx / 30.0f;

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

      glKosBeginFrame ();

      glKosMatrixDirty ();

      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      object_render_immediate (&view, skybox, &obj_orient, &lights, 0);

      glKosMatrixDirty ();

      glPushMatrix ();
      glTranslatef (0, 0, 4);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (&view, tube, &obj_orient, &lights, 0);
      glPopMatrix ();
      
#if 0
      glKosMatrixDirty ();
      
      glPushMatrix ();
      glTranslatef (2, 0, 6);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (&view, tube, &obj_orient, &lights, 0);
      glPopMatrix ();

      glKosMatrixDirty ();

      glPushMatrix ();
      glTranslatef (-2, 0, 6);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (&view, tube, &obj_orient, &lights, 0);
      glPopMatrix ();
#endif

      glKosMatrixDirty ();
      
      glKosFinishList ();

      glPushMatrix ();
      glTranslatef (0, 0, 4);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (&view, tube, &obj_orient, &lights, 1);
      glPopMatrix ();

#if 0
      glKosMatrixDirty ();

      glPushMatrix ();
      glTranslatef (2, 0, 6);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (&view, tube, &obj_orient, &lights, 1);
      glPopMatrix ();

      glKosMatrixDirty ();

      glPushMatrix ();
      glTranslatef (-2, 0, 6);
      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (&view, tube, &obj_orient, &lights, 1);
      glPopMatrix ();
#endif

      glKosFinishFrame ();
      
      rot1 += 0.05;
    }

  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
