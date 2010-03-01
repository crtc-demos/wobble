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
float eye_pos[] = {0.0, 0.0, 15};

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
    2 * 1024 * 1024,	/* Vertex buffer size 2M.  */
    0,			/* Use DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

object *
allocate_tube (int rows, int segments)
{
  int r;
  strip *prev_strip = NULL;
  
  for (r = 0; r < rows; r++)
    {
      strip *newstrip = strip_cons (prev_strip, segments * 2 + 2,
				    ALLOC_GEOMETRY | ALLOC_NORMALS);
      prev_strip = newstrip;
    }
  
  return object_create_default (prev_strip);
}

void
fill_tube_data (object *obj, int rows, int segments, float rot1)
{
  int r, s;
  strip *strlist = obj->striplist;
  
  for (r = 0; r < rows; r++)
    {
      float (*str)[][3];
      float (*norm)[][3];
      float ang0 = rot1 + 4 * (float) r / (float) rows;
      float ang1 = rot1 + 4 * (float) (r + 1) / (float) rows;
      float mag = 1.0 + 0.35 * fsin (ang0);
      float mag1 = 1.0 + 0.35 * fsin (ang1);

      assert (strlist);
      
      str = strlist->start;
      norm = strlist->normals;
      strlist->inverse = 1;
      
      for (s = 0; s <= segments; s++)
        {
	  float ang = 2 * M_PI * (float) s / (float) segments;
	  float cosang = fcos (ang);
	  float sinang = fsin (ang);
	  float norma[3];
	  
	  (*str)[s * 2][0] = mag * cosang;
	  (*str)[s * 2][1] = (2.0 * (float) r / (float) rows) - 1.0;
	  (*str)[s * 2][2] = mag * sinang;

	  (*str)[s * 2 + 1][0] = mag1 * cosang;
	  (*str)[s * 2 + 1][1] = (2.0 * (float) (r + 1) / (float) rows) - 1.0;
	  (*str)[s * 2 + 1][2] = mag1 * sinang;
	  
	  norma[0] = cosang;
	  norma[1] = 0.35 * fcos (ang0);
	  norma[2] = sinang;
	  
	  vec_normalize (&(*norm)[s * 2][0], &norma[0]);
	  
	  norma[1] = 0.35 * fcos (ang1);

	  vec_normalize (&(*norm)[s * 2 + 1][0], &norma[0]);
	}
      
      strlist = strlist->next;
    }
}

#define ROWS 16
#define SEGMENTS 32

#define DUCKS 24

static float rot1 = 0;

static float pos[DUCKS][3];
static float rotaxis[DUCKS][3];
static float velocity[DUCKS][3];
static char duck_colour[DUCKS];

uint8 dmabuffers[2][1024*1024] __attribute__((aligned(32)));

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  object *tube /*= allocate_tube (ROWS, SEGMENTS)*/;
  fakephong_info f_phong;
  viewpoint view;
  object_orientation obj_orient;
  lighting lights;
  int active = 0;
  int spawnctr = 0;

  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_NTSC_IL, PM_RGB565);

  init_pvr ();

  /*pvr_set_vertbuf (PVR_LIST_OP_POLY, dmabuffers[0], 1024 * 1024);
  pvr_set_vertbuf (PVR_LIST_TR_POLY, dmabuffers[1], 1024 * 1024);*/

  highlight = pvr_mem_malloc (256 * 256);
  fakephong_highlight_texture (highlight, 256, 256, 10.0f);

  tube = load_object ("/rd/duck_lo.stp");

  f_phong.highlight = highlight;
  f_phong.xsize = 256;
  f_phong.ysize = 256;
  f_phong.intensity = 128;

  tube->fake_phong = &f_phong;

  object_set_ambient (tube, 64, 0, 0);
  object_set_pigment (tube, 255, 0, 0);
  object_set_clipping (tube, 0);
  
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

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (eye_pos[0], eye_pos[1], eye_pos[2],	/* Eye position.  */
	     0.0,  0.0,  0.0,				/* Centre.  */
	     0.0,  1.0,  0.0);				/* Up.  */
  
  glGetFloatv (GL_MODELVIEW_MATRIX, &camera[0][0]);
  vec_transpose_rotation (&invcamera[0][0], &camera[0][0]);
  
  view.projection = &projection;
  view.camera = &camera;
  view.inv_camera_orientation = &invcamera;
  memcpy (&view.eye_pos[0], &eye_pos[0], 3 * sizeof (float));
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
      int i;

      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
	pushx = st->joyx / 25.0;
	pushy = (st->joyy / 25.0) + 2.0;
      MAPLE_FOREACH_END ()

      glKosBeginFrame ();

      glKosMatrixDirty ();

      for (i = 0; i < active; i++)
        {
	  if (duck_colour[i])
	    {
	      object_set_ambient (tube, 64, 64, 0);
	      object_set_pigment (tube, 255, 255, 0);
	    }
	  else
	    {
	      object_set_ambient (tube, 64, 32, 64);
	      object_set_pigment (tube, 255, 127, 255);
	    }
	
	  glPushMatrix ();
	  glTranslatef (pos[i][0], pos[i][1], pos[i][2]);
	  glRotatef (rot1, rotaxis[i][0], rotaxis[i][1], rotaxis[i][2]);
	  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
	  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
	  object_render_immediate (&view, tube, &obj_orient, &lights, 0);
	  glPopMatrix ();
	}
            
      glKosFinishList ();

#if 0
      for (i = 0; i < active; i++)
        {
	  glPushMatrix ();
	  glTranslatef (pos[i][0], pos[i][1], pos[i][2]);
	  glRotatef (rot1, rotaxis[i][0], rotaxis[i][1], rotaxis[i][2]);
	  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
	  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
	  object_render_immediate (&view, tube, &obj_orient, &lights, 1);
	  glPopMatrix ();
	}
#endif

      glKosFinishFrame ();
      
      for (i = 0; i < active; i++)
        {
	  pos[i][0] += velocity[i][0];
	  pos[i][1] += velocity[i][1];
	  pos[i][2] += velocity[i][2];
	  
	  velocity[i][1] -= 0.01;
	  if (pos[i][1] < -4)
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

  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
