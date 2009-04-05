#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glu.h>
#ifdef DREAMCAST_KOS
#include <kos.h>
#include <png/png.h>
#else
#include <GL/glut.h>
#endif

#include "vector.h"

#define EYE_POS 0.0, 0.0, -4.5

#ifdef DREAMCAST_KOS
extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);
#endif

#include "readpng.h"
#include "transform.h"

//#define MULTITEXTURE 1
#undef MULTITEXTURE

//#define MODULATE_HACK 1
#undef MODULATE_HACK

void dump_grid_from_texture_matrix(int front);

#ifndef DREAMCAST_KOS
void enable_reflection_str(void)
{
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_NV);
        
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);
}
#endif

void setup_texture_matrix(int front)
{
  GLfloat mapMatrix[16] = {
    0.5, 0,   0,  0,
    0,   0.5, 0,  0,
    0,   0,   1,  0,
    0.5, 0.5, 0,  1
  };
  GLfloat projectMatrix[16] = {
    1,  0,  0,  0,
    0,  1,  0,  0,
    0,  0,  1,  1,
    0,  0,  0,  0
  };
  GLfloat diffFrontMatrix[16] = {
    -1,  0,  0,  0,
     0,  1,  0,  0,
     0,  0,  1,  0,
     0,  0,  1,  1
  };
  GLfloat diffBackMatrix[16] = {
   -1,   0,  0,  0,
    0,   1,  0,  0,
    0,   0, -1,  0,
    0,   0,  1,  1
  };
  GLfloat mv[16], ilmv[16];

  glGetFloatv(GL_MODELVIEW_MATRIX, mv);  
  mv[3]  = 0;
  mv[7]  = 0;
  mv[11] = 0;
  mv[12] = 0;
  mv[13] = 0;
  mv[14] = 0;
  mv[15] = 1;
  transform_invertGLmatrix(ilmv, mv);

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMultMatrixf(mapMatrix);
  glMultMatrixf(projectMatrix);
  if (front)
    glMultMatrixf(diffFrontMatrix);
  else
    glMultMatrixf(diffBackMatrix);

  glMultMatrixf(ilmv);
/*  dump_grid_from_texture_matrix(front);*/
}

void dump_grid_from_texture_matrix(int front)
{
  GLfloat tmat[16];
  GLfloat pt[4], tpt[4];
  int i, j;
  
  printf("side: %s\n", front ? "front" : "back");
  
  glGetFloatv(GL_TEXTURE_MATRIX, tmat);
  
  for (j=0; j<12; j++)
  {
    float phi = (j-6)*M_PI/12.0;
    pt[1] = sin(phi);
    float rad = cos(phi);
    for (i=0; i<20; i++)
    {
      float theta = i*2*M_PI/20.0;
      pt[0] = rad*sin(theta);
      pt[2] = rad*cos(theta);
     /* float length = sqrt(xpos*xpos + ypos*ypos + zpos*zpos);*/
      pt[3] = 1;
      transform_pointGL(tpt, tmat, pt);
      fprintf (stderr, "%f %f\n", tpt[0], tpt[1]);
    }
   /* putc('\n', stdout);*/
  }
}

int front_txr, back_txr;

#ifndef DREAMCAST_KOS
void initialise(void)
{
  GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat mat_diffuse[] = {0.4f, 0.4f, 0.4f, 1.0f};
  GLfloat lmodel_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
  GLfloat light_amb[] = {0.2f, 0.2f, 0.2f, 1.0f};
  GLfloat light_diff[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat light_pos[] = {-3, 3, 0, 1};
  
  char* imgpixels;
  int x, y, alpha;

#ifdef MULTITEXTURE
#ifdef MODULATE_HACK
  extern int modulate_hack;
  modulate_hack = 1;
  imgpixels = readpng_image("front4a.png", &x, &y, &alpha);
#else
  imgpixels = readpng_image("front4.png", &x, &y, &alpha);
#endif
#else
  imgpixels = readpng_image("front4a.png", &x, &y, &alpha);
#endif
  front_txr = readpng_bindgl2d(imgpixels, x, y, alpha);

  /* clamp front txr */
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  imgpixels = readpng_image("back4a.png", &x, &y, &alpha);
  back_txr = readpng_bindgl2d(imgpixels, x, y, alpha);

  /* clamp back txr
   * (I think clamping is per-texture-unit not per-texture?)
   */
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  fprintf(stderr, "Bound front_txr to %d, back_txr to %d\n", front_txr,
    back_txr);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diff);

  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_LEQUAL);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);
  
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  /* texture unit 0 */
#ifdef MULTITEXTURE
  glActiveTexture(GL_TEXTURE0);
#endif
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_2D);
  enable_reflection_str();

#ifdef MULTITEXTURE  
  /* texture unit 1 */
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_2D);
#ifdef MODULATE_HACK
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#else
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
#endif
  enable_reflection_str();
#else
  glEnable(GL_ALPHA_TEST);
#endif

  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 60.0,
  /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 30.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, -5.0,     /* eye is here */
            0.0, 0.0, 0.0,      /* center is here */
            0.0, 1.0, 0.0);     /* up is in positive Y direction */
}
#endif /* DREAMCAST_KOS.  */

float rot = 0.0;
int curobj = 0;

#ifndef DREAMCAST_KOS
#define MAXOBJ 3

void display(void)
{
  GLUquadricObj* cyl = gluNewQuadric();
  GLfloat texcol[4] = {0.0, 0.0, 0.0, 0.0};
#ifndef MULTITEXTURE
  int repeat;
#endif

  gluQuadricNormals(cyl, GLU_SMOOTH);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef MULTITEXTURE
  /* setup front texture */
  glActiveTexture(GL_TEXTURE0);
  setup_texture_matrix(0);
  glBindTexture(GL_TEXTURE_2D, front_txr);

  /* setup back texture */
  glActiveTexture(GL_TEXTURE1);
  setup_texture_matrix(1);
  glBindTexture(GL_TEXTURE_2D, back_txr);
#endif

  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texcol);
  
#ifndef MULTITEXTURE
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.5f);
  
  for (repeat=0; repeat<2; repeat++)
  {
    setup_texture_matrix(repeat);
    glBindTexture(GL_TEXTURE_2D, repeat==0 ? front_txr : back_txr);
#endif

    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();

    glRotatef(rot, 0, 1, 0);
    glRotatef(rot/2.0, 1, 0, 0);

  /*  glColor3f(1.0, 0.6, 0.0);*/
    glColor4f(0.0, 1.0, 0.0, 1.0);

    switch (curobj)
    {
      case 0:
      glTranslatef(0, 0, -1.5);
      gluCylinder(cyl, 0.5, 0.5, 3, 40, 4);
      gluQuadricOrientation(cyl, GLU_INSIDE);
      gluDisk(cyl, 0, 0.5, 40, 1);
      gluQuadricOrientation(cyl, GLU_OUTSIDE);
      glTranslatef(0, 0, 3);
      gluDisk(cyl, 0, 0.5, 40, 1);
      break;

      case 1:
      glScalef(-1, 1, 1);
      glutSolidTeapot(1.5);
      break;

      case 2:
      glutSolidTorus(0.7, 1.3, 20, 40);
      break;
    }

    glPopMatrix();

#ifndef MULTITEXTURE
  }
  glDisable(GL_ALPHA_TEST);
#endif
    
  gluDeleteQuadric(cyl);
  
  glutSwapBuffers();
}

void idle(void)
{
  /* well, we are idle, so be idle */
  rot += 0.1;
  glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
    case ' ':
    curobj++;
    if (curobj >= MAXOBJ) curobj = 0;
    break;
    
    case 'q':
    case 27:
    exit(0);
    break;
  }
}

int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Parabolik");
  initialise();
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  glutMainLoop();

  return 0;
}
#else /* DREAMCAST_KOS.  */

static matrix_t camera __attribute__((aligned(32)));
static matrix_t invcamera __attribute__((aligned(32)));
static matrix_t transform __attribute__((aligned(32)));
static matrix_t rotate __attribute__((aligned(32)));

static void
fake_light (float vertex[3], float normal[3], int front)
{
  float light[3] = { 0.2357, 0.2357, 0.9428 };
  float eye[3] = { EYE_POS };
  float vertex4[4], x_vertex[4], x_normal[4];
  float eye_to_vertex[3], reflection[4], tmp[3];
  float eye_norm_dot;
  float incidence, normalized_normal[3];
  int r, g, b;
  GLfloat unrot[4], x, y, z;
  float x_f, y_f, z_f;
  float x_b, y_b, z_b;
  
  vec_normalize (normalized_normal, normal);

  /* Find the normal in eye space.  */
  vec_transform (x_normal, (float *) &rotate[0][0], normalized_normal);

  incidence = vec_dot (&light[0], &x_normal[0]);
  
  if (incidence < 0)
    incidence = 0;
  
  r = 64 + 191 * incidence;
  g = 64 + 191 * incidence;
  b = 64 + 191 * incidence;
  
  glColor4ub (r, g, b, 0);
  
  /* We need the vertex in eye space.  This duplicates work!  */
  memcpy (vertex4, vertex, sizeof (float) * 3);
  vertex4[3] = 0.0;
  vec_transform (x_vertex, (float *) &transform[0][0], vertex4);
  vec_sub (eye_to_vertex, x_vertex, eye);
  vec_normalize (eye_to_vertex, eye_to_vertex);
  
  eye_norm_dot = vec_dot (eye_to_vertex, x_normal);
  vec_scale (tmp, x_normal, -eye_norm_dot * 2.0);
  vec_add (reflection, eye_to_vertex, tmp);
  /*x = reflection[0];
  y = reflection[1];
  z = reflection[2];
  w = 1.0;
  mat_load ((matrix_t *) &unrotate[0][0]);
  mat_trans_nodiv (x, y, z, w);
  glKosMatrixDirty ();*/
  reflection[3] = 0.0;
  vec_transform (unrot, (float *) &invcamera[0][0], reflection);
  x = unrot[0];
  y = unrot[1];
  z = unrot[2];
  /* (x, y, z, w) should now be the reflection vector in object space (r_o).
    
    for the front face:
    
    d_o - r_o = [ k.x  k.y  k ]
    
    for the back face:
    
    -d_o - r_o = [ -k.x  -k.y  k ]
    
    for both, calculate lhs, then divide x, y by z.  */
  
  if (front)
    {
      x_f = 0.0 - x;
      y_f = 0.0 - y;
      z_f = -1.0 - z;

      if (z_f != 0.0)
	{
	  x_f = 0.5 + 0.5 * (x_f / z_f);
	  y_f = 0.5 + 0.5 * (y_f / z_f);
	}

      glTexCoord2f (x_f, y_f);
    }
  else
    {
      x_b = 0.0 - x;
      y_b = 0.0 - y;
      z_b = 1.0 - z;

      if (z_b != 0.0)
	{
	  x_b = 0.5 + 0.5 * (x_b / z_b);
	  y_b = 0.5 - 0.5 * (y_b / z_b);
	}

      glTexCoord2f (x_b, y_b);
    }
}

void
render_torus (float outer, float inner, int front)
{
  int major;
  const int minor_steps = 20, major_steps = 40;
  float major_ang = 0.0, major_incr = 2.0 * M_PI / major_steps;
  float first_row[minor_steps + 1][2][3], prev_row[minor_steps + 1][2][3];
  
  for (major = 0; major <= major_steps; major++, major_ang += major_incr)
    {
      int minor;
      float maj_x = fcos (major_ang) * outer;
      float maj_y = fsin (major_ang) * outer;
      float minor_ang = 0.0, minor_incr = 2.0 * M_PI / minor_steps;
      float first[2][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };
      float last[2][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };

      for (minor = 0; minor <= minor_steps; minor++, minor_ang += minor_incr)
        {
	  float min[2][3], orig_min_x;
	  
	  min[1][2] = fcos (minor_ang);
	  min[0][2] = min[1][2] * inner;
	  orig_min_x = fsin (minor_ang) * inner;
	  
	  /* 2D rotation is just:
	  
	     [ cos T  -sin T ] (x)
	     [ sin T   cos T ] (y).
	     
	     y is zero, so the corresponding terms disappear.  */

	  min[1][0] = fcos (major_ang) * orig_min_x;
	  min[0][0] = min[1][0] + maj_x;
	  min[1][1] = fsin (major_ang) * orig_min_x;
	  min[0][1] = min[1][1] + maj_y;
	  
	  if (major == 0)
	    {
	      memcpy (&prev_row[minor][0][0], &min[0][0], 6 * sizeof (float));
	      memcpy (&first_row[minor][0][0], &min[0][0], 6 * sizeof (float));
	      continue;
	    }
	  
	  if (minor == 0)
	    memcpy (&first[0], &min[0], 6 * sizeof (float));
	  else
	    {
	      int minor_wrapped = (minor == minor_steps) ? 0 : minor;

	      /* Polygon is formed from:
	           prev_row[minor-1]  ,-> last_xyz
		        v            /       v
		   prev_row[minor] -'	  min_xyz.  */
	      glBegin (GL_TRIANGLE_STRIP);
	      
	      //glColor4ub (255, 255, 255, 0);
	      fake_light (prev_row[minor - 1][0], prev_row[minor - 1][1],
			  front);
	      glVertex3fv ((GLfloat *) &prev_row[minor - 1][0]);
	      
	      //glColor4ub (255, 0, 0, 0);
	      fake_light (prev_row[minor][0], prev_row[minor][1], front);
	      glVertex3fv ((GLfloat *) &prev_row[minor][0]);
	      
	      if (major == major_steps)
	        {
		  //glColor4ub (255, 255, 0, 0);
		  fake_light (first_row[minor - 1][0], first_row[minor - 1][1],
			      front);
		  glVertex3fv ((GLfloat *) &first_row[minor - 1][0]);
		  
		  //glColor4ub (0, 255, 0, 0);
		  fake_light (first_row[minor_wrapped][0],
			      first_row[minor_wrapped][1], front);
		  glVertex3fv ((GLfloat *) &first_row[minor_wrapped][0]);
		}
	      else
	        {
		  //glColor4ub (255, 255, 0, 0);
		  fake_light (last[0], last[1], front);
		  glVertex3fv (&last[0][0]);

		  //glColor4ub (0, 255, 0, 0);
		  if (minor == minor_steps)
		    {
		      fake_light (first[0], first[1], front);
	              glVertex3fv (&first[0][0]);
		    }
		  else
		    {
		      fake_light (min[0], min[1], front);
		      glVertex3fv (&min[0][0]);
		    }
		}
	      
	      glEnd ();

	      memcpy (&prev_row[minor - 1][0][0], &last[0][0],
		      6 * sizeof (float));
	    }

	  memcpy (&last[0][0], &min[0][0], 6 * sizeof (float));
	}

      memcpy (&prev_row[minor_steps][0][0], &prev_row[0][0],
	      6 * sizeof (float));
    }
}

static void
init_pvr (void)
{
  pvr_init_params_t params = {
    { PVR_BINSIZE_16,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_0,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_16 },	/* Punch-thrus.  */
    512 * 1024,		/* Vertex buffer size 512K.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

int
main (int argc, char* argv[])
{
  int cable_type, quit = 0;
  float rot1 = 0.0, rot2 = 0.0;
  kos_img_t front_txr, back_txr;
  pvr_ptr_t texaddr;
  GLuint texture[2];
  int i;
  
  cable_type = vid_check_cable ();
  
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);
  
  init_pvr ();

  glKosInit ();
  
  png_to_img ("/rd/front4a.png", PNG_MASK_ALPHA, &front_txr);
  texaddr = pvr_mem_malloc (front_txr.w * front_txr.h * 2);
  pvr_txr_load_kimg (&front_txr, texaddr, PVR_TXRLOAD_INVERT_Y);
  kos_img_free (&front_txr, 0);
  
  glGenTextures (2, &texture[0]);
  
  glBindTexture (GL_TEXTURE_2D, texture[0]);
  glKosTex2D (GL_ARGB1555_TWID, front_txr.w, front_txr.h, texaddr);

  png_to_img ("/rd/back4a.png", PNG_MASK_ALPHA, &back_txr);
  texaddr = pvr_mem_malloc (back_txr.w * back_txr.h * 2);
  pvr_txr_load_kimg (&back_txr, texaddr, PVR_TXRLOAD_INVERT_Y);
  kos_img_free (&back_txr, 0);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glBindTexture (GL_TEXTURE_2D, texture[1]);
  glKosTex2D (GL_ARGB1555_TWID, back_txr.w, back_txr.h, texaddr);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glEnable (GL_TEXTURE_2D);
  glShadeModel (GL_SMOOTH);
  glClearDepth (1.0f);
  glDepthFunc (GL_LEQUAL);
  
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (60.0,			/* Field of view in degrees.  */
		  640.0 / 480.0,	/* Aspect ratio.  */
		  1.0,			/* Z near.  */
		  50.0);		/* Z far.  */

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (EYE_POS,			/* Eye position.  */
	     0.0,   0.0,   0.0,		/* Centre.  */
	     0.0,   1.0,   0.0);	/* Up.  */
  
  glGetFloatv (GL_MODELVIEW_MATRIX, &camera[0][0]);
  
  invcamera[0][0] = camera[0][0];
  invcamera[0][1] = camera[1][0];
  invcamera[0][2] = camera[2][0];
  invcamera[0][3] = 0.0;

  invcamera[1][0] = camera[0][1];
  invcamera[1][1] = camera[1][1];
  invcamera[1][2] = camera[2][1];
  invcamera[1][3] = 0.0;

  invcamera[2][0] = camera[0][2];
  invcamera[2][1] = camera[1][2];
  invcamera[2][2] = camera[2][2];
  invcamera[2][3] = 0.0;

  invcamera[3][0] = 0.0;
  invcamera[3][1] = 0.0;
  invcamera[3][2] = 0.0;
  invcamera[3][3] = 1.0;
  
  printf ("Inverted camera matrix:\n");
  for (i = 0; i < 4; i++)
    {
      printf ("[ %f %f %f %f ]\n", invcamera[0][i], invcamera[1][i],
	      invcamera[2][i], invcamera[3][i]);
    }
  
  while (!quit)
    {
      glKosBeginFrame ();
      
      glPushMatrix ();
      
      #if 1
      glRotatef (rot1, 0.0, 1.0, 0.0);
      rot1 += 0.7;
      glRotatef (rot2, 1.0, 0.0, 0.0);
      rot2 += 0.29;
      #else
      glRotatef (rot1, 0.0, 0.0, 1.0);
      rot1 += 0.7;
      #endif
      
      if (rot1 >= 360)
        rot1 -= 360;

      if (rot2 >= 360)
        rot2 -= 360;
      
      glGetFloatv (GL_MODELVIEW_MATRIX, &transform[0][0]);
      
      /* Rotation part of transformation.  */
      rotate[0][0] = transform[0][0];
      rotate[1][0] = transform[1][0];
      rotate[2][0] = transform[2][0];
      rotate[3][0] = 0.0;
      
      rotate[0][1] = transform[0][1];
      rotate[1][1] = transform[1][1];
      rotate[2][1] = transform[2][1];
      rotate[3][1] = 0.0;
      
      rotate[0][2] = transform[0][2];
      rotate[1][2] = transform[1][2];
      rotate[2][2] = transform[2][2];
      rotate[3][2] = 0.0;
      
      rotate[0][3] = 0.0;
      rotate[1][3] = 0.0;
      rotate[2][3] = 0.0;
      rotate[3][3] = 1.0;
      
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()
      
      /* Render front.  */
      glBindTexture (GL_TEXTURE_2D, texture[0]);
      glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      render_torus (1.0, 0.6, 1);
      
      glKosFinishList ();

      /* Render back.  */
      glBindTexture (GL_TEXTURE_2D, texture[1]);
      glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      render_torus (1.0, 0.6, 0);
      
      glPopMatrix ();
      
      glKosFinishFrame ();
    }
  
  glKosShutdown ();
  
  pvr_shutdown ();
  
  vid_shutdown ();
  /* Make dc-load look nicer.  */
  vid_init (DM_640x480_PAL_IL, PM_RGB565);

  return 0;
}

#endif /* DREAMCAST_KOS.  */


