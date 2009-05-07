#include <math.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vector.h"

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
float acc[YSIZE][XSIZE];
float normals[YSIZE][XSIZE][3];

void
init_grid (void)
{
  unsigned int i, j;
  
  for (j = 0; j < YSIZE; j++)
    for (i = 0; i < XSIZE; i++)
      pos[j][i] = vel[j][i] = acc[j][i] = 0.0;
}

float rot1 = 0.0;

void
update_grid (void)
{
  const float timestep = 0.05;
  const float spring = 0.17;
  const float mass = 0.04;
  const float drag = 0.0045;
  int i, j;

  const float yfac = sin (M_PI / 3.0);
  const float iscale = 1.0 / (float) XSIZE, jscale = yfac / (float) YSIZE;
  const float ihalf = iscale / 2.0;

  pos[19][15] = fsin (rot1) / 2;
  
  rot1 += 0.05;
  if (rot1 > 2 * M_PI)
    rot1 -= 2 * M_PI;

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

void
draw_water (int clockwise)
{
  unsigned int i, j;
  const float yfac = sin (M_PI / 3.0);
  const float iscale = 1.0 / (float) XSIZE, jscale = yfac / (float) YSIZE;
  const float ihalf = iscale / 2.0;
  
  glPushMatrix ();
  glScalef (3, 0.5, 3);
  
  for (j = 1; j < YSIZE - 2; j++)
    {
      float js = (float) j * jscale;
      
      glBegin (GL_TRIANGLE_STRIP);
      
      for (i = 1; i < XSIZE - 2; i++)
        {
	  float is = (float) i * iscale + (float) j * ihalf;

	  set_colour (normals[j][i]);
          if (!clockwise && i == 1)
	    glVertex3f (is, pos[j][i], js);
	  glVertex3f (is, pos[j][i], js);
	  set_colour (normals[j + 1][i]);
	  glVertex3f (is + ihalf, pos[j + 1][i], js + jscale);
	}
      
      glEnd ();
    }

  glPopMatrix ();
}

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

static void
init_pvr (void)
{
  pvr_init_params_t params = {
    { PVR_BINSIZE_32,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_0,	/* Translucent polygons.  */
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

  cable_type = vid_check_cable ();

  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);

  init_pvr ();

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
  gluPerspective (45.0,			/* Field of view in degrees.  */
		  640.0 / 480.0,	/* Aspect ratio.  */
		  1.0,			/* Z near.  */
		  50.0);		/* Z far.  */

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (-0.866, 0.75, -0.5,		/* Eye position.  */
	      0.866, 0.0,   0.5,		/* Centre.  */
	      0.0,   1.0,   0.0);		/* Up.  */

  init_grid ();

  while (!quit)
    {
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()

      glKosBeginFrame ();
      
      update_grid ();
      draw_water (1);

      glKosFinishList ();

      glKosFinishFrame ();
    }

  glKosShutdown ();
  
  pvr_shutdown ();
  
  vid_shutdown ();

  return 0;
}
