#include <kos.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <math.h>

#include "vector.h"

#define NUMPETAL 64

static float
bezier (float t, float p0, float p1, float p2, float p3)
{
  float it = 1.0f - t;
  return it * it * it * p0 + 3.0 * it * it * t * p1
	 + 3.0f * it * t * t * p2 + t * t * t * p3;
}

void
petal (float lhs[4][3], float rhs[4][3])
{
  int tt;
  float ol[3] = {0, 0, 0};

  glBegin (GL_TRIANGLE_STRIP);

  for (tt = 0; tt <= 20; tt++)
  {
    float l[3], r[3], d1[3], d2[3], nl[3];
    float t = tt * 0.05;

    l[0] = bezier (t, lhs[0][0], lhs[1][0], lhs[2][0], lhs[3][0]);
    l[1] = bezier (t, lhs[0][1], lhs[1][1], lhs[2][1], lhs[3][1]);
    l[2] = bezier (t, lhs[0][2], lhs[1][2], lhs[2][2], lhs[3][2]);
    r[0] = bezier (t, rhs[0][0], rhs[1][0], rhs[2][0], rhs[3][0]);
    r[1] = bezier (t, rhs[0][1], rhs[1][1], rhs[2][1], rhs[3][1]);
    r[2] = bezier (t, rhs[0][2], rhs[1][2], rhs[2][2], rhs[3][2]);
    //vec_sub (&d1[0], &r[0], &l[0]);
    //vec_sub (&d2[0], &ol[0], &l[0]);
    //vec_cross (&nl[0], &d2[0], &d1[0]);
    // left and right normals are parallel
    //glNormal3f (nl[0], nl[1], nl[2]);
    glVertex3f (l[0], l[1], l[2]);
    glVertex3f (r[0], r[1], r[2]);
    //printf ("%f %f %f\n", l[0], l[1], l[2]);
    memcpy (&ol[0], &l[0], 3 * sizeof (float));
  }
  glEnd ();
}

int
petalfrompoints (float lhs[4][3], float rhs[4][3], float in[4][3])
{
  float mid[3], vec_a[3], vec_a2[3], vec_b[3], across[3];
  
  mid[0] = (in[0][0] + in[2][0]) / 2.0f;
  mid[1] = (in[0][1] + in[2][1]) / 2.0f;
  mid[2] = (in[0][2] + in[2][2]) / 2.0f;
  
  vec_sub (&vec_a2[0], &in[1][0], &mid[0]);
  vec_normalize (&vec_a[0], &vec_a2[0]);

  vec_sub (&vec_b[0], &in[2][0], &in[0][0]);
  
  vec_cross (&across[0], &vec_a[0], &vec_b[0]);
  vec_normalize (&across[0], &across[0]);
  
/*  across.x *= 0.5;
  across.y *= 0.5;
  across.z *= 0.5;*/
  
  memcpy (&lhs[0], &in[0], 3 * sizeof (float));
  memcpy (&rhs[0], &in[0], 3 * sizeof (float));
  memcpy (&lhs[3], &in[2], 3 * sizeof (float));
  memcpy (&rhs[3], &in[2], 3 * sizeof (float));

  lhs[1][0] = (in[1][0] + vec_a2[0] + across[0] + in[0][0]) * 0.5f;
  lhs[1][1] = (in[1][1] + vec_a2[1] + across[1] + in[0][1]) * 0.5f;
  lhs[1][2] = (in[1][2] + vec_a2[2] + across[2] + in[0][2]) * 0.5f;
  
  lhs[2][0] = (in[1][0] + vec_a2[0] + across[0] + in[2][0]) * 0.5f;
  lhs[2][1] = (in[1][1] + vec_a2[1] + across[1] + in[2][1]) * 0.5f;
  lhs[2][2] = (in[1][2] + vec_a2[2] + across[2] + in[2][2]) * 0.5f;
  
  rhs[1][0] = (in[1][0] + vec_a2[0] - across[0] + in[0][0]) * 0.5f;
  rhs[1][1] = (in[1][1] + vec_a2[1] - across[1] + in[0][1]) * 0.5f;
  rhs[1][2] = (in[1][2] + vec_a2[2] - across[2] + in[0][2]) * 0.5f;
  
  rhs[2][0] = (in[1][0] + vec_a2[0] - across[0] + in[2][0]) * 0.5f;
  rhs[2][1] = (in[1][1] + vec_a2[1] - across[1] + in[2][1]) * 0.5f;
  rhs[2][2] = (in[1][2] + vec_a2[2] - across[2] + in[2][2]) * 0.5f;
  
  return vec_dot (&vec_a2[0], &across[0]) < 0 ? 0 : 1;
}

int
main (int argc, char* argv[])
{
  float amt = 0.0f;
  int i, j;
  float petl[4][3], petr[4][3];
  float in[3][3], ***buffer;
  float freq[NUMPETAL][5][3], amp[NUMPETAL][5][3], disp[NUMPETAL][5][3];
  float toang = 0.01;
  int cable_type;
  pvr_init_params_t params = {
    { PVR_BINSIZE_32,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_0,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_0 },	/* Punch-thrus.  */
    512 * 1024,		/* Vertex buffer size 512K.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  int quit = 0;
  float colours[6][3] = {
    {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f}
  };
  
  cable_type = vid_check_cable ();
  
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);
  
  pvr_init (&params);
  
  glKosInit ();

  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glShadeModel (GL_SMOOTH);
  glClearDepth (1.0f);
  glDepthFunc (GL_LEQUAL);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (45.0,			/* FOV in degrees.  */
		  640.0 / 480.0,	/* Aspect ratio.  */
		  1.0,			/* Z near.  */
		  50.0);		/* Z far.  */
  //glFrustum (-3.0f, 3.0f, -3.0 * aspect, 3.0 * aspect, 5.0, 60.0);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (0.0, 0.0, -8.0,	/* Eye position.  */
	     0.0, 0.0,  0.0,	/* Centre.  */
	     0.0, 1.0,  0.0);	/* Up.  */

  buffer = calloc (sizeof (float**), NUMPETAL);

  for (j = 0; j < NUMPETAL; j++)
    {
      buffer[j] = calloc (sizeof (float*), 16);

      for (i = 0; i < 16; i++)
	buffer[j][i] = calloc (sizeof (float), 3);

      for (i = 0; i < 5; i++)
	{
	  freq[j][i][0] = 0.1 * (float) rand () / (float) RAND_MAX;
	  freq[j][i][1] = 0.1 * (float) rand () / (float) RAND_MAX;
	  freq[j][i][2] = 0.1 * (float) rand () / (float) RAND_MAX;
	  disp[j][i][0] = 2.0 * M_PI * (float) rand () / (float) RAND_MAX;
	  disp[j][i][1] = 2.0 * M_PI * (float) rand () / (float) RAND_MAX;
	  disp[j][i][2] = 2.0 * M_PI * (float) rand () / (float) RAND_MAX;
	  amp[j][i][0] = 1.5 * (float) rand () / (float) RAND_MAX;
	  amp[j][i][1] = 1.5 * (float) rand () / (float) RAND_MAX;
	  amp[j][i][2] = 1.5 * (float) rand () / (float) RAND_MAX;
	}
    }
  
  while (!quit)
    {
      int k;

      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()
      
      glKosBeginFrame ();
      
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glPushMatrix ();
      //glTranslatef (0.0f, 0.0f, -8.0f);

      //glRotatef (amt / 5, 0.0f, 1.0f, 0.0f);

      for (k = 0; k < NUMPETAL; k++)
	{
	  float xa = 0, ya = 0, za = 0;

	  for (j = 0; j < 5; j++)
	    {
              xa += amp[k][j][0] * fsin (toang * freq[k][j][0] + disp[k][j][0]);
              ya += amp[k][j][1] * fsin (toang * freq[k][j][1] + disp[k][j][1]);
              za += amp[k][j][2] * fsin (toang * freq[k][j][2] + disp[k][j][2]);
	    }

	  buffer[k][i][0] = xa;
	  buffer[k][i][1] = ya;
	  buffer[k][i][2] = za;

	  memcpy (&in[0], &buffer[k][i][0], 3 * sizeof (float));
	  memcpy (&in[1], &buffer[k][(i - 7) & 15][0], 3 * sizeof (float));
	  memcpy (&in[2], &buffer[k][(i - 15) & 15][0], 3 * sizeof (float));

          glColor3f (colours[k % 6][0], colours[k % 6][1], colours[k % 6][2]);

	  petalfrompoints (petl, petr, in);
	  petal (petl, petr);
	}

      amt += 2.5f;
      toang += 2.5;
      i++;
      if (i > 15)
        i = 0;

      glPopMatrix ();

      glKosFinishFrame ();
    }
  
  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();

  return 0;
}

