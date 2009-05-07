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
#include "distort.h"
#include "perlin.h"

int draw_vectors = 100;

float eye_pos[4] = { 0.0, 0.0, 0.0, 1.0 };

extern uint16 *vram_s;

/* Blobby thing.  */
GLfloat ***sphere;
int nstrip;
int stripl;
float blob_phase = 0.0;

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
static matrix_t projection __attribute__((aligned(32)));
static matrix_t modelview __attribute__((aligned(32)));

static void
crap_box (int x, int y, int colour)
{
  int i, j;
  
  for (j = 0; j < 2; j++)
   for (i = 0; i < 2; i++)
     vram_s[(y + j) * 640 + x + i] = colour;
}

static void
box (float *vertex, int colour)
{
  float xx_vertex[4];

  glGetFloatv (GL_PROJECTION_MATRIX, &projection[0][0]);
  glGetFloatv (GL_MODELVIEW_MATRIX, &modelview[0][0]);

  //vec_mat_apply (&tmp[0], &projection[0][0], &modelview[0][0]);
  
  //vec_transform (x_vertex, (float *) &modelview[0][0], vertex);
  vec_transform (xx_vertex, (float *) &projection[0][0], vertex);
  
  xx_vertex[0] = 320 + 320 * (xx_vertex[0] / xx_vertex[3]) - 1;
  xx_vertex[1] = 240 - 240 * (xx_vertex[1] / xx_vertex[3]) - 1;
  xx_vertex[2] = xx_vertex[2] / xx_vertex[3];
  
  if (xx_vertex[0] >= 0 && xx_vertex[0] < (640 - 2)
      && xx_vertex[1] >= 0 && xx_vertex[1] < (480 - 2)
      && xx_vertex[2] > 0)
    crap_box (xx_vertex[0], xx_vertex[1], colour);
}

static void
parabolic_texcoords (float *texc, float vertex[3], float normal[3], int front)
{
  //float light[3] = { 0.2357, 0.2357, 0.9428 };
  float vertex4[4], x_vertex[4], x_normal[4];
  float eye_to_vertex[3], reflection[4], tmp[3];
  float temp[4], along;
  float eye_norm_dot;
  float incidence, normalized_normal[4];
  int r, g, b;
  GLfloat unrot[4], x, y, z;
  float x_f, y_f, z_f;
  float x_b, y_b, z_b;
  float c_eyepos[4];

  vec_normalize (normalized_normal, normal);

  /* Find the normal in eye space.  */
  normalized_normal[3] = 1.0;
  vec_transform (x_normal, (float *) &rotate[0][0], normalized_normal);

 /* incidence = vec_dot (&light[0], &x_normal[0]);
  
  if (incidence < 0)
    incidence = 0;*/
  
  /*r = 64 + 191 * incidence;
  g = 64 + 191 * incidence;
  b = 64 + 191 * incidence;
  
  glColor4ub (r, g, b, 0);
  glColor4ub (255, 255, 255, 0);*/
  
  /* We need the vertex in eye space.  This duplicates work!  */
  memcpy (vertex4, vertex, sizeof (float) * 3);
  vertex4[3] = 1.0;
  vec_transform (x_vertex, (float *) &transform[0][0], vertex4);

  vec_transform (&c_eyepos[0], &camera[0][0], &eye_pos[0]);
  vec_sub (eye_to_vertex, &c_eyepos[0], &x_vertex[0]);
  vec_normalize (eye_to_vertex, eye_to_vertex);
  
  eye_norm_dot = vec_dot (eye_to_vertex, &x_normal[0]);
  vec_scale (tmp, x_normal, 2.0 * eye_norm_dot);

  vec_sub (reflection, tmp, eye_to_vertex);
  
  //dbgio_printf ("ref length: %f\n", (double) vec_length (reflection));

#if 0
  if (draw_vectors > 0)
    {
     /* dbgio_printf ("%f %f %f\n", (double) eye_to_vertex[0],
				  (double) eye_to_vertex[1],
				  (double) eye_to_vertex[2]); */

      dbgio_printf ("e2v . norm = %f  norm . refl = %f\n",
        (double) eye_norm_dot, (double) vec_dot (x_normal, reflection));

      for (along = 0.0; along < 0.25; along += 0.005)
	{
	  int colour = 0x001f | ((int) (along * 255) << 5);
	  vec_scale (temp, x_normal, along);
	  vec_add (&temp[0], x_vertex, &temp[0]);
	  box (temp, colour);
	}

      for (along = 0.0; along < 0.25; along += 0.005)
	{
	  int colour = 0xf800 | ((int) (along * 255) << 5);

	  /*vec_scale (temp, eye_to_vertex, along);
	  vec_add (&temp[0], x_vertex, &temp[0]);
	  box (temp, colour);*/

	  colour &= ~0xf800;

	  vec_scale (temp, reflection, along);
	  vec_add (&temp[0], x_vertex, &temp[0]);
	  box (temp, colour);
	}
      draw_vectors--;
    }
#endif

  /*x = reflection[0];
  y = reflection[1];
  z = reflection[2];
  w = 1.0;
  mat_load ((matrix_t *) &unrotate[0][0]);
  mat_trans_nodiv (x, y, z, w);
  glKosMatrixDirty ();*/
  reflection[3] = 1.0;
  vec_transform (unrot, (float *) &invcamera[0][0], reflection);
  x = unrot[0];
  y = unrot[1];
  z = unrot[2];

  //glColor4ub (128 + 127 * x, 128 + 127 * y, 128 + 127 * z, 0);

  /*dbgio_printf ("x_norm len: %f  ref len: %f  unrot len: %f (%f, %f, %f)\n",
                (double) vec_length (x_normal),
                (double) vec_length (reflection),
		(double) vec_length (unrot),
		(double) unrot[0],
		(double) unrot[1],
		(double) unrot[2]);*/

  /* (x, y, z, w) should now be the reflection vector in object space (r_o).
    
    for the front face:
    
    d_o - r_o = [ k.x  k.y  k ]
    
    for the back face:
    
    -d_o - r_o = [ -k.x  -k.y  k ]
    
    for both, calculate lhs, then divide x, y by z.  */
    
  if (front)
    {
      x_f = x;
      y_f = -y;
      z_f = -1.0 - z;

      /*if (z_f >= 0.0)
        z_f = -0.0001;*/

      x_f = 0.5 + 0.5 * (x_f / z_f);
      y_f = 0.5 + 0.5 * (y_f / z_f);

      texc[0] = x_f;
      texc[1] = y_f;
      texc[2] = z;
    }
  else
    {
      x_b = x;
      y_b = y;
      z_b = 1.0 - z;

      /*if (z_b <= 0.0)
        z_b = 0.0001;*/

      x_b = 0.5 + 0.5 * (x_b / z_b);
      y_b = 0.5 + 0.5 * (y_b / z_b);

      texc[0] = x_b;
      texc[1] = y_b;
      texc[2] = z;
    }
}

static void
grab_transform (void)
{
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
}

void
render_torus (float outer, float inner, int front)
{
  int major;
  const int minor_steps = 15, major_steps = 30;
  float major_ang = 0.0, major_incr = 2.0 * M_PI / major_steps;
  float first_row[minor_steps + 1][2][3], prev_row[minor_steps + 1][2][3];
  
  /* We need this to calculate shininess properly.  */
  grab_transform ();
  
  for (major = 0; major <= major_steps; major++, major_ang += major_incr)
    {
      int minor;
      float fsin_major = fsin (major_ang);
      float fcos_major = fcos (major_ang);
      float maj_x = fcos_major * outer;
      float maj_y = fsin_major * outer;
      float minor_ang = 0.0, minor_incr = 2.0 * M_PI / minor_steps;
      float first[2][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };
      float last[2][3] = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };

      for (minor = 0; minor <= minor_steps; minor++, minor_ang += minor_incr)
        {
	  float fsin_minor = fsin (minor_ang);
	  float fcos_minor = fcos (minor_ang);
	  float min[2][3], orig_min_x;
	  
	  min[1][0] = fcos_major * fsin_minor;
	  min[1][1] = fsin_major * fsin_minor;
	  min[1][2] = fcos_minor;
	  
	  /* 2D rotation is just:
	  
	     [ cos T  -sin T ] (x)
	     [ sin T   cos T ] (y).
	     
	     y is zero, so the corresponding terms disappear.  */

	  orig_min_x = fsin_minor * inner;
	  min[0][0] = fcos_major * orig_min_x + maj_x;
	  min[0][1] = fsin_major * orig_min_x + maj_y;
	  min[0][2] = fcos_minor * inner;

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
	      float texcoords[12];
	      GLfloat *vptr[4];
	      int vnum = 0;

	      /* Polygon is formed from:
	           prev_row[minor-1]  ,-> last_xyz
		        v            /       v
		   prev_row[minor] -'	  min_xyz.  */
	      
	      //glColor4ub (255, 255, 255, 0);
	      parabolic_texcoords (&texcoords[0],
				   prev_row[minor - 1][0],
				   prev_row[minor - 1][1],
				   front);
	      vptr[vnum++] = (GLfloat *) &prev_row[minor - 1][0];
	      
	      //glColor4ub (255, 0, 0, 0);
	      parabolic_texcoords (&texcoords[3],
				   prev_row[minor][0],
				   prev_row[minor][1],
				   front);
	      vptr[vnum++] = (GLfloat *) &prev_row[minor][0];
	      
	      if (major == major_steps)
	        {
		  //glColor4ub (255, 255, 0, 0);
		  parabolic_texcoords (&texcoords[6],
				       first_row[minor - 1][0],
				       first_row[minor - 1][1],
				       front);
		  vptr[vnum++] = (GLfloat *) &first_row[minor - 1][0];
		  
		  //glColor4ub (0, 255, 0, 0);
		  parabolic_texcoords (&texcoords[9],
				       first_row[minor_wrapped][0],
				       first_row[minor_wrapped][1],
				       front);
		  vptr[vnum++] = (GLfloat *) &first_row[minor_wrapped][0];
		}
	      else
	        {
		  //glColor4ub (255, 255, 0, 0);
		  parabolic_texcoords (&texcoords[6], last[0], last[1], front);
		  vptr[vnum++] = &last[0][0];

		  //glColor4ub (0, 255, 0, 0);
		  if (minor == minor_steps)
		    {
		      parabolic_texcoords (&texcoords[9], first[0], first[1],
					   front);
		      vptr[vnum++] = &first[0][0];
		    }
		  else
		    {
		      parabolic_texcoords (&texcoords[9], min[0], min[1],
					   front);
		      vptr[vnum++] = &min[0][0];
		    }
		}

	      if ((front && (texcoords[2] > 0 || texcoords[5] > 0
			     || texcoords[8] > 0 || texcoords[11] > 0))
		  || (!front && (texcoords[2] < 0 || texcoords[5] < 0
				 || texcoords[8] < 0 || texcoords[11] < 0)))
		{
		  glBegin (GL_TRIANGLE_STRIP);
		  glTexCoord2f (texcoords[0], texcoords[1]);
		  glVertex3fv (vptr[0]);
		  glTexCoord2f (texcoords[3], texcoords[4]);
		  glVertex3fv (vptr[1]);
		  glTexCoord2f (texcoords[6], texcoords[7]);
		  glVertex3fv (vptr[2]);
		  glTexCoord2f (texcoords[9], texcoords[10]);
		  glVertex3fv (vptr[3]);
		  glEnd ();
		}

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
render_blob (GLfloat ***data, int numstrips, int striplength, int front)
{
  int i;
  
  grab_transform ();
  
  for (i = 0; i < numstrips; i++)
    {
      int j;
      float texcoords[3 * striplength];
      GLfloat v[striplength][3];
      int all_positive = 1;
      int all_negative = 1;
      
      for (j = 0; j < striplength; j++)
        {
	  GLfloat x;
	  memcpy (&v[j], data[i][j], sizeof (float) * 3);
	  x = perlin_noise_2D (15 + 3 * v[j][1] + blob_phase * 0.3,
			       15 + 2 * v[j][0] + blob_phase * 0.4, 1) * 0.35;
	  v[j][0] += x * 0.5;
	  v[j][1] += x * 0.25;
	  v[j][2] += x * 0.15;
	  parabolic_texcoords (&texcoords[j * 3], v, data[i][j], front);
	  if (texcoords[j * 3 + 2] < 0)
	    all_positive = 0;
	  if (texcoords[j * 3 + 2] > 0)
	    all_negative = 0;
	}
      
      if ((front && !all_negative) || (!front && !all_positive))
        {
	  glBegin (GL_TRIANGLE_STRIP);
          for (j = 0; j < striplength; j++)
	    {
	      glTexCoord2f (texcoords[j * 3], texcoords[j * 3 + 1]);
	      glVertex3fv (&v[j][0]);
	    }
	  glEnd ();
	}
    }
}

/* This should use real clipping.  */
static void
render_cube (GLuint *textures)
{
#define CUBE_SCALE 100
  int i;
  
  glColor4ub (255, 255, 255, 0);
  
  for (i = 0; i < 6; i++)
    {
      int j, k;
      const float delta = 2 * CUBE_SCALE / 3.0;
      
      glBindTexture (GL_TEXTURE_2D, textures[i]);
      for (j = 0; j < 3; j++)
        {
	  float jl = (float) j / 3.0;
	  float jh = (float) (j + 1) / 3.0;

          for (k = 0; k < 3; k++)
	    {
	      float kl = (float) k / 3.0;
	      float kh = (float) (k + 1) / 3.0;
	      float x = 0, y = 0, z = 0;
	      float jx = 0, jy = 0, jz = 0;
	      float kx = 0, ky = 0, kz = 0;

	      switch (i)
	        {
		case 1:
		  x = CUBE_SCALE;
		  y = kl * 2 * CUBE_SCALE - CUBE_SCALE;
		  z = jl * 2 * CUBE_SCALE - CUBE_SCALE;
		  jz = delta;
		  ky = delta;
		  break;
		
		case 0:
		  x = -CUBE_SCALE;
		  y = kl * 2 * CUBE_SCALE - CUBE_SCALE;
		  z = CUBE_SCALE - jl * 2 * CUBE_SCALE;
		  jz = -delta;
		  ky = delta;
		  break;

		case 2:
                  x = kl * 2 * CUBE_SCALE - CUBE_SCALE;
		  y = -CUBE_SCALE;
		  z = jl * 2 * CUBE_SCALE - CUBE_SCALE;
		  jz = delta;
		  kx = delta;
		  break;
		
		case 3:
                  x = CUBE_SCALE - kl * 2 * CUBE_SCALE;
		  y = CUBE_SCALE;
		  z = jl * 2 * CUBE_SCALE - CUBE_SCALE;
		  jz = delta;
		  kx = -delta;
		  break;
		
		case 5:
                  x = CUBE_SCALE - jl * 2 * CUBE_SCALE;
		  y = kl * 2 * CUBE_SCALE - CUBE_SCALE;
		  z = CUBE_SCALE;
		  jx = -delta;
		  ky = delta;
		  break;

		case 4:
                  x = jl * 2 * CUBE_SCALE - CUBE_SCALE;
		  y = kl * 2 * CUBE_SCALE - CUBE_SCALE;
		  z = -CUBE_SCALE;
		  jx = delta;
		  ky = delta;
		  break;
		}

	      glBegin (GL_QUADS);
	      glTexCoord2f (jl, 1.0 - kl);
	      glVertex3f (x, y, z);
	      glTexCoord2f (jh, 1.0 - kl);
	      glVertex3f (x + jx, y + jy, z + jz);
	      glTexCoord2f (jh, 1.0 - kh);
	      glVertex3f (x + jx + kx, y + jy + ky, z + jz + kz);
	      glTexCoord2f (jl, 1.0 - kh);
	      glVertex3f (x + kx, y + ky, z + kz);
	      glEnd ();
	    }
	}
    }
#undef CUBE_SCALE
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
  float rot1 = 0.0, rot2 = 0.0, rot3 = 0.0;
  kos_img_t front_txr, back_txr;
  pvr_ptr_t texaddr;
  GLuint texture[8];
  int blendfunc = 2;
  float eye_rot = 0;

  cable_type = vid_check_cable ();
  
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);
  
  init_pvr ();

  glKosInit ();

  sphere = mkspheredata (3, &nstrip, &stripl);
  
  png_to_img ("/rd/sky1.png", PNG_MASK_ALPHA, &front_txr);
  texaddr = pvr_mem_malloc (front_txr.w * front_txr.h * 2);
  pvr_txr_load_kimg (&front_txr, texaddr, PVR_TXRLOAD_INVERT_Y);
  kos_img_free (&front_txr, 0);
  
  glGenTextures (8, &texture[0]);
  
  glBindTexture (GL_TEXTURE_2D, texture[0]);
  glKosTex2D (GL_ARGB1555_TWID, front_txr.w, front_txr.h, texaddr);

  png_to_img ("/rd/sky2o.png", PNG_MASK_ALPHA, &back_txr);
  texaddr = pvr_mem_malloc (back_txr.w * back_txr.h * 2);
  pvr_txr_load_kimg (&back_txr, texaddr, PVR_TXRLOAD_INVERT_Y);
  kos_img_free (&back_txr, 0);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glBindTexture (GL_TEXTURE_2D, texture[1]);
  glKosTex2D (GL_ARGB1555_TWID, back_txr.w, back_txr.h, texaddr);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  glBindTexture (GL_TEXTURE_2D, texture[2]);
  texaddr = pvr_mem_malloc (256 * 256 * 2);
  png_to_texture ("/rd/sky3.png", texaddr, PNG_NO_ALPHA);
  glKosTex2D (GL_RGB565_TWID, 256, 256, texaddr);

  glBindTexture (GL_TEXTURE_2D, texture[3]);
  texaddr = pvr_mem_malloc (256 * 256 * 2);
  png_to_texture ("/rd/sky4.png", texaddr, PNG_NO_ALPHA);
  glKosTex2D (GL_RGB565_TWID, 256, 256, texaddr);

  glBindTexture (GL_TEXTURE_2D, texture[4]);
  texaddr = pvr_mem_malloc (256 * 256 * 2);
  png_to_texture ("/rd/sky5.png", texaddr, PNG_NO_ALPHA);
  glKosTex2D (GL_RGB565_TWID, 256, 256, texaddr);

  glBindTexture (GL_TEXTURE_2D, texture[5]);
  texaddr = pvr_mem_malloc (256 * 256 * 2);
  png_to_texture ("/rd/sky6.png", texaddr, PNG_NO_ALPHA);
  glKosTex2D (GL_RGB565_TWID, 256, 256, texaddr);

  glBindTexture (GL_TEXTURE_2D, texture[6]);
  texaddr = pvr_mem_malloc (256 * 256 * 2);
  png_to_texture ("/rd/sky7.png", texaddr, PNG_NO_ALPHA);
  glKosTex2D (GL_RGB565_TWID, 256, 256, texaddr);

  glBindTexture (GL_TEXTURE_2D, texture[7]);
  texaddr = pvr_mem_malloc (256 * 256 * 2);
  png_to_texture ("/rd/sky8.png", texaddr, PNG_NO_ALPHA);
  glKosTex2D (GL_RGB565_TWID, 256, 256, texaddr);
  
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
  
  while (!quit)
    {
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        {
	  if (st->buttons & CONT_START)
	    quit = 1;
	  
	  eye_pos[0] = st->joyx / 25.0;
	  eye_pos[1] = st->joyy / 25.0;
	  eye_pos[2] = 5 * sin (eye_rot);
	  
	  if (st->buttons & CONT_A)
	    blendfunc = 0;
	  else if (st->buttons & CONT_B)
	    blendfunc = 1;
	  else if (st->buttons & CONT_X)
	    blendfunc = 2;
	}
      MAPLE_FOREACH_END ()
      
      draw_vectors = 10;
      
      glLoadIdentity ();
      gluLookAt (eye_pos[0], eye_pos[1], eye_pos[2],	/* Eye position.  */
		 0.0,   0.0,   0.0,			/* Centre.  */
		 0.0,   1.0,   0.0);			/* Up.  */

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

      /*dbgio_printf ("inverted camera orthogonality: %f %f %f\n",
		    (double) vec_dot (invcamera[0], invcamera[1]),
		    (double) vec_dot (invcamera[1], invcamera[2]),
		    (double) vec_dot (invcamera[2], invcamera[0]));*/

      /*dbgio_printf ("Inverted camera matrix:\n");
      for (i = 0; i < 4; i++)
	{
	  dbgio_printf ("[ %f %f %f %f ]\n",
                	(double) invcamera[0][i], (double) invcamera[1][i],
	        	(double) invcamera[2][i], (double) invcamera[3][i]);
	}*/

      glKosBeginFrame ();

      /*glPushMatrix ();
      glTranslatef (-camera[3][0], -camera[3][1], -camera[3][2]);*/
      render_cube (&texture[2]);
      /*glPopMatrix ();*/
      
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
      
      eye_rot += 0.05;
      if (eye_rot >= 2 * M_PI)
        eye_rot -= 2 * M_PI;
      
      blob_phase += 0.05;
      if (blob_phase >= 24 * M_PI)
        blob_phase -= 24 * M_PI;
      
      /* Render front.  */
      glBindTexture (GL_TEXTURE_2D, texture[0]);
      glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      #if 1
      render_blob (sphere, nstrip, stripl, 1);
      #else
      render_torus (1.0, 0.6, 1);
      #endif
            
      glPushMatrix ();
      glRotatef (rot3, 1.0, 0.0, 0.0);
      glTranslatef (2.9, 0.0, 0.0);
      render_torus (0.8, 0.5, 1);
      glPopMatrix ();
      glPushMatrix ();
      glRotatef (-rot3, 1.0, 0.0, 0.0);
      glTranslatef (-2.9, 0.0, 0.0);
      render_torus (0.8, 0.5, 1);
      glPopMatrix ();
      
      glKosFinishList ();

      /* Render back.  */
      glBindTexture (GL_TEXTURE_2D, texture[1]);
      glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      if (blendfunc == 0)
        glBlendFunc (GL_SRC_ALPHA, GL_ZERO);
      else if (blendfunc == 1)
        glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
      else
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
      #if 1
      //glDisable (GL_TEXTURE_2D);
      render_blob (sphere, nstrip, stripl, 0);
      //glEnable (GL_TEXTURE_2D);
      #else
      render_torus (1.0, 0.6, 0);
      #endif

      glPushMatrix ();
      glRotatef (rot3, 1.0, 0.0, 0.0);
      glTranslatef (2.9, 0.0, 0.0);
      render_torus (0.8, 0.5, 0);
      glPopMatrix ();
      glPushMatrix ();
      glRotatef (-rot3, 1.0, 0.0, 0.0);
      glTranslatef (-2.9, 0.0, 0.0);
      render_torus (0.8, 0.5, 0);
      glPopMatrix ();

      rot3 += 1;
      if (rot3 >= 360)
        rot3 -= 360;

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


