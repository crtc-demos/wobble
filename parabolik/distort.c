
/*  Triangle subdivision perlin noise blob distortion demo
 *  (c) Julian Brown 2002
 *
 *  Some code nicked from glut examples...
 */

#ifndef DREAMCAST_KOS
#include <GL/glut.h>
#define SQRT(X) sqrt(X)
#else
#include <kos.h>
#include <GL/gl.h>
#include <GL/glu.h>
#define glNormal3fv(X) /* nop */
#define glMaterialfv(X,Y,Z) /* nop */
#define glLightfv(X,Y,Z) /* nop */
#define SQRT(X) fsqrt(X)
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "perlin.h"

/* Define float type for vector.h.  */
#define FLOAT_TYPE GLfloat
#include "vector.h"

GLfloat light_diffuse[] =
{0.9, 0.7, 0.2, 1.0};
GLfloat light_position[] =
{1.0, 1.0, 1.0, 0.0};

GLfloat light2_diffuse[] =
{0.0, 0.1, 0.4, 1.0};
GLfloat light2_position[] =
{-1.0, -1.0, -0.5, 0.0};

GLfloat mat_specular[] = {0.3f, 0.4f, 0.5f, 1.0f};
GLfloat mat_shininess[] = {20.0f};

GLfloat*** sphdata;
int nstrip;
int stripl;
GLfloat rot;

#define T_TOP 0
#define T_LEFT 1
#define T_FRONT 2
#define T_RIGHT 3
#define T_BACK 4
#define T_BOTTOM 5

void mid(GLfloat* d, GLfloat* s1, GLfloat* s2)
{
  d[0] = (s1[0]+s2[0])/2.0;
  d[1] = (s1[1]+s2[1])/2.0;
  d[2] = (s1[2]+s2[2])/2.0;
}

#define COPYVEC(A,B) memcpy((A), (B), sizeof(GLfloat)*3)

GLfloat*** mkspheredata(int depth, int* nstrips, int* stripl)
{
  GLfloat points[][3] =
  { {0.0, 1.0, 0.0},  /* top */
    {-1.0, 0.0, 0.0},  /* left */
    {0.0, 0.0, -1.0},  /* front */
    {1.0, 0.0, 0.0},   /* right */
    {0.0, 0.0, 1.0},  /* back */
    {0.0, -1.0, 0.0} };  /* bottom */
  GLfloat*** strips, ***strops;
    
  int striplength = 4;
  int numstrips = 4;
  int i;
  int recur;

  strips = calloc(sizeof(GLfloat**), numstrips);
  for (i=0; i<numstrips; i++)
  {
    int j;
    strips[i] = calloc(sizeof(GLfloat*), striplength);
    for (j=0; j<striplength; j++)
    {
      strips[i][j] = calloc(sizeof(GLfloat), 3);
    }
  }

  COPYVEC(strips[0][0], &points[T_BOTTOM]);
  COPYVEC(strips[0][1], &points[T_LEFT]);
  COPYVEC(strips[0][2], &points[T_FRONT]);
  COPYVEC(strips[0][3], &points[T_TOP]);

  COPYVEC(strips[1][0], &points[T_BOTTOM]);
  COPYVEC(strips[1][1], &points[T_FRONT]);
  COPYVEC(strips[1][2], &points[T_RIGHT]);
  COPYVEC(strips[1][3], &points[T_TOP]);
  
  COPYVEC(strips[2][0], &points[T_BOTTOM]);
  COPYVEC(strips[2][1], &points[T_RIGHT]);
  COPYVEC(strips[2][2], &points[T_BACK]);
  COPYVEC(strips[2][3], &points[T_TOP]);
  
  COPYVEC(strips[3][0], &points[T_BOTTOM]);
  COPYVEC(strips[3][1], &points[T_BACK]);
  COPYVEC(strips[3][2], &points[T_LEFT]);
  COPYVEC(strips[3][3], &points[T_TOP]);
  
  for (recur=0; recur<depth; recur++)
  {
    int newlength = ((striplength-2)*2)+2;
    strops = calloc(sizeof(GLfloat**), numstrips*2);
    for (i=0; i<numstrips*2; i++)
    {
      int j;
      strops[i] = calloc(sizeof(GLfloat*), newlength);
      for (j=0; j<newlength; j++)
      {
        strops[i][j] = calloc(sizeof(GLfloat), 3);
      }
    }

    for (i=0; i<numstrips; i++)
    {
      GLfloat edge[3], side[3], lastedge[3], lastside[3];
      int j, alt = 0;
      int strop_a = 0, strop_b = 0;
      int k = i*2;

  /*    mid(side, strips[i][0], strips[i][1]);
      mid(edge, strips[i][0], strips[i][2]);

      normalise(&side[0]);
      normalise(&edge[0]);*/

      for (j=0; j<striplength-2; j++)
      {
        COPYVEC(lastedge, edge);
        COPYVEC(lastside, side);

        mid(side, strips[i][j], strips[i][j+1]);
        mid(edge, strips[i][j], strips[i][j+2]);

        vec_normalize (&side[0], &side[0]);
        vec_normalize (&edge[0], &edge[0]);

        if (alt)  // 1st, 3rd, etc.
        {
          COPYVEC(strops[k][strop_a++], lastside);
          COPYVEC(strops[k][strop_a++], lastedge);
          COPYVEC(strops[k][strop_a++], side);

          COPYVEC(strops[k+1][strop_b++], side);
          COPYVEC(strops[k+1][strop_b++], edge);
        }
        else  // 0th, 2nd, 4th eth
        {
          COPYVEC(strops[k][strop_a++], strips[i][j]);

          COPYVEC(strops[k+1][strop_b++], side);
          COPYVEC(strops[k+1][strop_b++], strips[i][j+1]);
        }
        
        alt = !alt;
      }

      /* finish off strop_a */
      mid(side, strips[i][striplength-2], strips[i][striplength-1]);
      vec_normalize (&side[0], &side[0]);

      COPYVEC(strops[k][strop_a++], strips[i][striplength-2]);
      COPYVEC(strops[k][strop_a++], side);

      /* finish off strop_b */
      COPYVEC(strops[k+1][strop_b++], side);
      COPYVEC(strops[k+1][strop_b++], strips[i][striplength-1]);
    }

    for (i=0; i<numstrips; i++)
    {
      int j;
      for (j=0; j<striplength; j++)
      {
        free(strips[i][j]);
      }
      free(strips[i]);
    }
    free(strips);
    
    strips = strops;

    numstrips *= 2;
    striplength = newlength;
  }
  
  *nstrips = numstrips;
  *stripl = striplength;
  
  return strips;
}

void delspheredata(GLfloat*** strips, int numstrips)
{
  int i;

  for (i=0; i<numstrips; i++)
  {
    int j;
    for (j=0; j<3; j++)
    {
      free(strips[i][j]);
    }
    free(strips[i]);
  }
  free(strips);
}

#ifdef DREAMCAST_KOS

// do dirty lighting calculation
void lightsource(GLfloat* pos, GLfloat* normal)
{
  GLfloat light_dir[][3] = {{0.58823526, 0.58823526, 0.58823526},
                            {-0.6666667, -0.6666667, -0.33333334}};
  GLfloat light_pos[][3] = {{1.0, 1.0, 1.0},
                            {-1.0, -1.0, -0.5}};
  GLfloat light_colour[][3] = {{0.9, 0.7, 0.2},
                               {0.0, 0.1, 0.4}};
  GLfloat spec_colour[] = {0.3, 0.4, 0.5};
  GLfloat shininess = 20.0;
  GLfloat camera[] = {0.0, 0.0, 1.0};
  GLfloat reflect[3], incident[3];
  int num = 2, i, j;
  // ambient light
  GLfloat colour[] = {0.2, 0.2, 0.2};
  GLfloat contrib, spec;

  // do simple diffuse shading
  for (i=0; i<num; i++)
  {
    float prop;
    float dp = vec_dot (normal, light_dir[i]);
    contrib = dp;
    if (contrib<0) contrib = 0;
    incident[0] = light_pos[i][0] - pos[0];
    incident[1] = light_pos[i][1] - pos[1];
    incident[2] = light_pos[i][2] - pos[2];
    prop = vec_dot (incident, normal);

    if (prop>0)
    {
      reflect[0] = -incident[0] + 2*prop*normal[0];
      reflect[1] = -incident[1] + 2*prop*normal[1];
      reflect[2] = -incident[2] + 2*prop*normal[2];
      vec_normalize (reflect, reflect);
      spec = vec_dot (reflect, camera);
    /*  fprintf(stderr, "spec=%f\n", spec);*/
      if (spec<0) spec=0;
      spec = powf(spec, shininess);
    }
    else spec = 0;

    for (j=0; j<3; j++)
    {
      colour[j] += (light_colour[i][j]-colour[j])*contrib;
      colour[j] += spec_colour[j]*spec;
      if (colour[j]>1.0) colour[j]=1.0;
    }
  }
  /* colour this point */
  glColor3fv(colour);
}
#endif

void mangle(GLfloat*** data, int numstrips, int striplength, GLfloat offx,
  GLfloat offy, GLfloat offz)
{
  int i;

  glPushMatrix();

/*  glLoadIdentity();*/
  glRotatef(rot*5, 0.5, 0.5, 0);

  for (i=0; i<numstrips; i++)
  {
    int j;
    glBegin(GL_TRIANGLE_STRIP);
    
    for (j=0; j<striplength; j++)
    {
      GLfloat n[3], x;
      COPYVEC(n, data[i][j]);
      n[0] += offx;
      n[1] += offy;
      n[2] += offz;
      x = perlin_noise_2D(15+3*n[1]+rot*0.3, 15+2*n[0]+rot*0.4, 1)*0.35;
      n[0] += x * 0.5;
      n[1] += x * 0.25;
      n[2] += x * 0.15;
#ifndef DREAMCAST_KOS
      glNormal3fv(data[i][j]);
#else
      lightsource(data[i][j], data[i][j]);
#endif
      glVertex3fv(n);
    }
    
    glEnd();
  }
  glPopMatrix();
}

#ifndef DREAMCAST_KOS
void
idle(void)
{
  static float time;

  time = glutGet(GLUT_ELAPSED_TIME) / 500.0;

  rot = time;
  glutPostRedisplay();
}

void
display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /* render sphere display list */
  mangle(sphdata, nstrip, stripl, 0.0, 0.0, 0.0); 
/*  base();*/
  glutSwapBuffers();
}
#endif

void
gfxinit(void)
{
#ifndef DREAMCAST_KOS
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light2_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light2_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
#endif
  glEnable(GL_DEPTH_TEST);
#ifndef DREAMCAST_KOS
  glEnable(GL_COLOR_MATERIAL);
#endif
  glEnable(GL_CULL_FACE);
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 40.0,
  /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
  glTranslatef(0.0, 0.0, -3.0);
}

#if 0
#ifndef DREAMCAST_KOS

/* glut main function */
int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Distort");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  gfxinit();
  sphdata = mkspheredata(6, &nstrip, &stripl);
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}

#else

void dcrun(void)
{
  uint8 c = maple_first_controller();
  GLfloat midx=0.0, midy=0.0;
  glKosInit();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, 640.0f/480.0f, 1.0, 50.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
  glTranslatef(0.0, 0.0, -0.5);

  while (1)
  {
    MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
      if (st->buttons & CONT_START) goto exit_from_while;
      midx = st->joyx/50.0;
      midy = -st->joyy/50.0;
    MAPLE_FOREACH_END()
    glKosBeginFrame();
    /* render sphere display list */
    mangle(sphdata, nstrip, stripl, midx, midy, 0.0);
    glKosFinishList();
    glKosFinishFrame();
    rot += 0.05;
  }
  exit_from_while:
  glKosShutdown(); 
}

KOS_INIT_FLAGS(INIT_DEFAULT);

/* DC main function */
int main(int argc, char* argv[])
{
  int cable_type;

  cable_type = vid_check_cable();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);

  pvr_init_defaults();

  gfxinit();
  sphdata = mkspheredata(3, &nstrip, &stripl);

  dcrun();

  return 0;
}

#endif
#endif
