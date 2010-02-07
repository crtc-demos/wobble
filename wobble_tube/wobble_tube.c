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

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

float light_pos[] = {5, -5, -15};
float light_updir[] = {0.0, 1.0, 0.0};
float eye_pos[] = {0.0, 0.0, -4.5};

static matrix_t camera __attribute__((aligned(32)));
static matrix_t invcamera __attribute__((aligned(32)));
static matrix_t projection __attribute__((aligned(32)));
static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));

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

object *
allocate_tube (int rows, int segments)
{
  int r;
  strip *prev_strip = NULL;
  
  for (r = 0; r < rows; r++)
    {
      strip *newstrip = malloc (sizeof (strip));
      float (*str)[][3] = malloc (3 * sizeof (float) * (segments * 2 + 2));
      float (*normals)[][3] = malloc (3 * sizeof (float) * (segments * 2 + 2));
      
      newstrip->start = str;
      newstrip->normals = normals;
      newstrip->texcoords = NULL;
      newstrip->length = segments * 2 + 2;
      newstrip->inverse = 0;
      newstrip->v_attrs = NULL;
      newstrip->s_attrs = NULL;
      newstrip->next = prev_strip;
      
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
	  
	  vec_normalize (&(*norm)[s * 2], &norma[0]);
	  
	  norma[1] = 0.35 * fcos (ang1);

	  vec_normalize (&(*norm)[s * 2 + 1], &norma[0]);
	}
      
      strlist = strlist->next;
    }
}

#define ROWS 16
#define SEGMENTS 32

float rot1 = 0;

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  object *tube = allocate_tube (ROWS, SEGMENTS);
  fakephong_info f_phong;
  
  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_NTSC_IL, PM_RGB565);

  init_pvr ();

  highlight = pvr_mem_malloc (256 * 256);
  fakephong_highlight_texture (highlight, 256, 256, 10.0f);

  f_phong.highlight = highlight;
  f_phong.xsize = 256;
  f_phong.ysize = 256;
  f_phong.intensity = 128;

  tube->fake_phong = &f_phong;

  object_set_ambient (tube, 64, 0, 0);
  object_set_pigment (tube, 255, 0, 0);
  
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

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (eye_pos[0], eye_pos[1], eye_pos[2],	/* Eye position.  */
	     0.0,  0.0,  0.0,				/* Centre.  */
	     0.0,  1.0,  0.0);				/* Up.  */
  
  glGetFloatv (GL_MODELVIEW_MATRIX, &camera[0][0]);
  vec_transpose_rotation (&invcamera[0][0], &camera[0][0]);
  
  /* glGenTextures (1, &texture[0]); */

  palette_grey_ramp ();

  glBlendFunc (GL_ONE, GL_ONE);
  
  pvr_set_bg_color (0.0, 0.0, 0.0);
  
  while (!quit)
    {
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()

      fill_tube_data (tube, ROWS, SEGMENTS, rot1);

      glKosBeginFrame ();

      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (tube, 0, mview, normxform, projection, camera,
			       invcamera, eye_pos, light_pos, light_updir);
      
      glKosFinishList ();

      object_render_immediate (tube, 1, mview, normxform, projection, camera,
			       invcamera, eye_pos, light_pos, light_updir);
      
      glKosFinishFrame ();
      
      rot1 += 0.05;
    }

  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
