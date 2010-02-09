#include <math.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vector.h"
#include "object.h"
#include "palette.h"

#define XSIZE 64
#define YSIZE 64

/*** we want to render water as triangles:
      ____________..
     /\  /\  /\  /\
    /__\/__\/__\/__\..
   /\  /\  /\  /\  /\
  /__\/__\/__\/__\/__\..

Realign this to a grid like so:
  _____________
  |\  |\  |\  |
  |__\|__\|__\|
  |\  |\  |\  |
  |__\|__\|__\|

so a point like this:

    1_____2
    /\   /\
   /  \ /  \
  6----*----3
   \  / \  /
    \/___\/
    5     4

has equidistant neighbours like this:

   1---2
   |   |
   6---*---3
       |   |
       5---4

***/

float pos[YSIZE][XSIZE];
float vel[YSIZE][XSIZE];
float normals[YSIZE][XSIZE][3];

void
init_grid (void)
{
  unsigned int i, j;
  
  for (j = 0; j < YSIZE; j++)
    for (i = 0; i < XSIZE; i++)
      pos[j][i] = vel[j][i] = 0.0;
}

float rot1 = 0.0;
float rot2 = 0.0;

void
update_grid (void)
{
  const float timestep = 0.05;
#if 0
  const float spring = 0.17;
  const float mass = 0.04;
  const float drag = 0.0045;
#else
  const float spring = 0.17;
  const float mass = 0.02;
  const float drag = 0.0045;
#endif
  int i, j;

  const float yfac = sin (M_PI / 3.0);
  const float iscale = 1.0 / (float) XSIZE, jscale = yfac / (float) YSIZE;
  const float ihalf = iscale / 2.0;

  pos[19][15] = fsin (rot1) / 2;
  pos[33][47] = fsin (rot2) / 3;
  
  rot1 += 0.07;
  if (rot1 > 2 * M_PI)
    rot1 -= 2 * M_PI;

  rot2 += 0.08;
  if (rot2 > 2 * M_PI)
    rot2 -= 2 * M_PI;

  for (j=1; j<YSIZE-1; j++)
  {
    for (i=1; i<XSIZE-1; i++)
    {
      double f, a, k1, k2;
      double restpos = (pos[j+1][i-1] +
                        pos[j+1][i] +
                        pos[j][i+1] +
                        pos[j-1][i+1] +
                        pos[j-1][i] +
                        pos[j][i-1]) / 6.0;

      f = (restpos - pos[j][i])*spring - drag*vel[j][i];
      a = f/mass;
      k1 = timestep * a;
      
      f = (restpos - pos[j][i])*spring - drag*(vel[j][i]+k1);
      a = f/mass;
      k2 = timestep * a;

      vel[j][i] = vel[j][i] + (k1+k2)/2;
      pos[j][i] = pos[j][i] + vel[j][i]*timestep;
    }
  }
  
  /* copy boundary */
  for (i=0; i<XSIZE; i++)
  {
    pos[0][i] = pos[1][i];
    pos[YSIZE-1][i] = pos[YSIZE-2][i];
  }
  for (j=0; j<YSIZE; j++)
  {
    pos[j][0] = pos[j][1];
    pos[j][XSIZE-1] = pos[j][XSIZE-2];
  }
  
  /* Calculate normals.  */
  for (j = 1; j < YSIZE - 1; j++)
    for (i = 1; i < XSIZE - 1; i++)
      {
        float ivec[3], jvec[3], norm[3];
	
	ivec[0] = iscale;
	ivec[1] = pos[j][i + 1] - pos[j][i];
	ivec[2] = 0;

	jvec[0] = ihalf;
	jvec[1] = pos[j + 1][i] - pos[j][i];
	jvec[2] = jscale;
	
	vec_cross (norm, jvec, ivec);
	vec_normalize (normals[j][i], norm);
      }
}

static void
set_colour (float norm[3])
{
  float light[3] = { 0.707, 0.707, 0 };
  float amt = vec_dot (light, norm);
  
  if (amt < 0.0)
    amt = 0.0;
  
  glColor3f (amt, amt, amt);
}

object *
allocate_water (unsigned int xsize, unsigned int ysize)
{
  strip *previous = NULL;
  int y;

  for (y = 1; y < ysize - 2; y++)
    {
      strip *newstrip = strip_cons (previous, (xsize - 3) * 2,
				    ALLOC_GEOMETRY | ALLOC_NORMALS);
      previous = newstrip;
    }

  return object_create_default (previous);
}

#define USE_GL 0

void
draw_water (object *obj, int clockwise)
{
  unsigned int i, j;
  const float yfac = sin (M_PI / 3.0);
  const float iscale = 1.0 / (float) XSIZE, jscale = yfac / (float) YSIZE;
  const float ihalf = iscale / 2.0;
  strip *mystrip = obj->striplist;

#if USE_GL
  glPushMatrix ();
  glScalef (3, 0.5, 3);
#endif
  
  for (j = 1; j < YSIZE - 2; j++)
    {
      float js = (float) j * jscale;
      float (*geom)[][3] = mystrip->start;
      float (*norm)[][3] = mystrip->normals;
      
      mystrip->inverse = 1;
      
      assert (mystrip);
      assert (geom);
      assert (norm);
      
      /*glBegin (GL_TRIANGLE_STRIP);*/

      for (i = 1; i < XSIZE - 2; i++)
        {
	  float is = (float) i * iscale + (float) j * ihalf;
	  int idx = (i - 1) * 2;

#if USE_GL
	  set_colour (normals[j][i]);
          if (!clockwise && i == 1)
	    glVertex3f (is, pos[j][i], js);
	  glVertex3f (is, pos[j][i], js);
	  set_colour (normals[j + 1][i]);
	  glVertex3f (is + ihalf, pos[j + 1][i], js + jscale);
#else
	  (*geom)[idx][0] = is * 3;
	  (*geom)[idx][1] = pos[j][i] * 0.5;
	  (*geom)[idx][2] = js * 3;
	  
	  (*norm)[idx][0] = normals[j][i][0];
	  (*norm)[idx][1] = normals[j][i][1];
	  (*norm)[idx][2] = normals[j][i][2];

	  (*geom)[idx + 1][0] = (is + ihalf) * 3;
	  (*geom)[idx + 1][1] = pos[j + 1][i] * 0.5;
	  (*geom)[idx + 1][2] = (js + jscale) * 3;

	  (*norm)[idx + 1][0] = normals[j + 1][i][0];
	  (*norm)[idx + 1][1] = normals[j + 1][i][1];
	  (*norm)[idx + 1][2] = normals[j + 1][i][2];
#endif
	}

#if USE_GL
      glEnd ();
#else
      mystrip = mystrip->next;
#endif
    }

#if USE_GL
  glPopMatrix ();
#endif
}

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

//float light_pos[] = {5, -5, -15};
float light_pos[] = {6.062, 2, 3.5};
float light_updir[] = {0.0, 1.0, 0.0};
float eye_pos[] = {-0.866, 0.75, -0.5};

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
    { PVR_BINSIZE_32,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_32,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_0 },	/* Punch-thrus.  */
    2 * 1024 * 1024,	/* Vertex buffer size 4MB.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  object *water = allocate_water (XSIZE, YSIZE);
  fakephong_info f_phong;
  envmap_dual_para_info envmap;
  kos_img_t front_txr, back_txr;
  pvr_ptr_t texaddr;

  cable_type = vid_check_cable ();

  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);

  init_pvr ();

#if 1
  highlight = pvr_mem_malloc (256 * 256);
  fakephong_highlight_texture (highlight, 256, 256, 10.0f);

  f_phong.highlight = highlight;
  f_phong.xsize = 256;
  f_phong.ysize = 256;
  f_phong.intensity = 128;
  
  water->fake_phong = &f_phong;
#else
  kmg_to_img ("/rd/sky1.kmg", &front_txr);
  texaddr = pvr_mem_malloc (front_txr.byte_count);
  pvr_txr_load_kimg (&front_txr, texaddr, PVR_TXRFMT_VQ_ENABLE);
  envmap.front_txr = texaddr;
  kos_img_free (&front_txr, 0);
  
  kmg_to_img ("/rd/sky2o.kmg", &back_txr);
  texaddr = pvr_mem_malloc (back_txr.byte_count);
  pvr_txr_load_kimg (&back_txr, texaddr, PVR_TXRFMT_VQ_ENABLE);
  envmap.back_txr = texaddr;
  kos_img_free (&back_txr, 0);
  
  envmap.xsize = front_txr.w;
  envmap.ysize = front_txr.h;
  
  water->env_map = &envmap;
#endif
  
  object_set_ambient (water, 0, 0, 64);
  object_set_pigment (water, 0, 0, 255);
  
  palette_grey_ramp ();

  vid_border_color (0, 0, 0);
  pvr_set_bg_color (0.0, 0.0, 0.0);
    
  glKosInit ();
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glEnable (GL_TEXTURE_2D);
  glShadeModel (GL_SMOOTH);
  glClearDepth (1.0f);
  glDepthFunc (GL_LEQUAL);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (30.0,			/* Field of view in degrees.  */
		  640.0 / 480.0,	/* Aspect ratio.  */
		  1.0,			/* Z near.  */
		  50.0);		/* Z far.  */

  glGetFloatv (GL_PROJECTION_MATRIX, &projection[0][0]);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (eye_pos[0], eye_pos[1], eye_pos[2],	/* Eye position.  */
	     0.866, 0.5,   0.5,				/* Centre.  */
	     0.0,   1.0,   0.0);			/* Up.  */

  glGetFloatv (GL_MODELVIEW_MATRIX, &camera[0][0]);
  vec_transpose_rotation (&invcamera[0][0], &camera[0][0]);

  init_grid ();

  while (!quit)
    {
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()

      glKosBeginFrame ();

      update_grid ();
      draw_water (water, 1);

      glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
      vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
      object_render_immediate (water, 0, mview, normxform, projection, camera,
			       invcamera, eye_pos, light_pos, light_updir);
      
      glKosFinishList ();

      object_render_immediate (water, 1, mview, normxform, projection, camera,
			       invcamera, eye_pos, light_pos, light_updir);

      glKosFinishFrame ();
    }

  glKosShutdown ();
  
  pvr_shutdown ();
  
  vid_shutdown ();

  return 0;
}
