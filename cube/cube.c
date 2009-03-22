/* Bump-mapped cube.  Bits stolen from Fredrik Ehnbom.  */

#include <math.h>
#include <stdlib.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>

/***

     3______ 2
     /:    /|
   7/_____/6|
    |0:.. |.|1
    | ,   | /
   4|_____|/5

***/

static GLfloat points[][3] =
{
  {-1.0, -1.0,  1.0},
  { 1.0, -1.0,  1.0},
  { 1.0,  1.0,  1.0},
  {-1.0,  1.0,  1.0},
  {-1.0, -1.0, -1.0},
  { 1.0, -1.0, -1.0},
  { 1.0,  1.0, -1.0},
  {-1.0,  1.0, -1.0},
};

static int tristrips[][4] =
{
  { 4, 7, 5, 6 },
  { 5, 6, 1, 2 },
  { 1, 2, 0, 3 },
  { 0, 3, 4, 7 },
  { 7, 3, 6, 2 },
  { 0, 4, 1, 5 }
};

static GLfloat face_normals[][3] =
{
  { 0.0,  0.0, -1.0 },
  { 1.0,  0.0,  0.0 },
  { 0.0,  0.0,  1.0 },
  {-1.0,  0.0,  0.0 },
  { 0.0,  1.0,  0.0 },
  { 0.0, -1.0,  0.0 }
};

/* The "orientation" vector is the zero-angle "up" direction for the
   texture.  For the current cube this is the vector from vertex 0 to
   vertex 1. ([u,v] corresponding to [0,0] -> [1,0])  */
static GLfloat face_orient[][3] =
{
  { 0.0,  1.0,  0.0 },
  { 0.0,  1.0,  0.0 },
  { 0.0,  1.0,  0.0 },
  { 0.0,  1.0,  0.0 },
  { 0.0,  0.0,  1.0 },
  { 0.0,  0.0, -1.0 }
};

static GLubyte colour[][4] =
{
  { 255, 255, 127, 0 },
  { 255, 127, 127, 0 },
  { 127, 255, 127, 0 },
  { 127, 127, 255, 0 },
  { 255, 127, 255, 0 },
  { 127, 255, 255, 0 }
};

/*
static GLubyte colour[][4] =
{
  { 255, 255, 255, 0 },
  { 255, 255, 255, 0 },
  { 255, 255, 255, 0 },
  { 255, 255, 255, 0 },
  { 255, 255, 255, 0 },
  { 255, 255, 255, 0 }
};
*/

/* Texture coordinates are "upside down", so V goes from 0 (top)
   to 1 (bottom).  */
static GLfloat texcoords[][2] =
{
  { 0.0, 0.0 },
  { 3.0, 0.0 },
  { 0.0, 3.0 },
  { 3.0, 3.0 }
};

extern uint8 rom[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (rom);

static pvr_ptr_t
load_bumpmap (const char *filename)
{
  void *data = malloc (128 * 128 * 2);
  pvr_ptr_t pvraddr;
  FILE *fp = fopen (filename, "rb");

  if (!fp)
    {
      printf ("Couldn't load bump map '%s'\n", filename);
      exit (1);
    }
  
  fread (data, 1, 128 * 128 * 2, fp);
  fclose (fp);
  
  pvraddr = pvr_mem_malloc (128 * 128 * 2);
  pvr_txr_load (data, pvraddr, 128 * 128 * 2);
  free (data);
  
  return pvraddr;
}

// T and Q defines the light source as polar coordinates where
// T is the elevation angle ranging from 0 to Pi/2 (90 degrees) and
// Q is the rotation angle ranging from 0 to Pi*2 (360 degrees).
// h is an intensity value specifying how much bumping should be done. 0 <= h <= 1
void
set_bump_direction (float T, float Q, float h)
{
  unsigned char Q2 = (unsigned char) ((Q / (2 * 3.1415927))* 255);
  unsigned char h2 = (unsigned char) (h * 255);

  unsigned char k1 = 255 - h2;
  unsigned char k2 = (unsigned char) (h2 * fsin(T));
  unsigned char k3 = (unsigned char) (h2 * fcos(T));

  /* The bump direction is encoded as an offset colour.  */
  glKosOffsetColor4ub (k2, k3, Q2, k1);
}

static void
init_pvr (void)
{
  pvr_init_params_t params = {
    { PVR_BINSIZE_8,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_0,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_8 },	/* Punch-thrus.  */
    512 * 1024,		/* Vertex buffer size 512K.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

static void
normalize (GLfloat *vec)
{
  GLfloat factor = 1.0 / fsqrt (vec[0] * vec[0] + vec[1] * vec[1]
				+ vec[2] * vec[2]);
  vec[0] *= factor;
  vec[1] *= factor;
  vec[2] *= factor;
}

static GLfloat
length (GLfloat *vec)
{
  return fsqrt (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

/* Maybe there's an instruction to help with this?  */
static void
cross (GLfloat *dst, GLfloat *a, GLfloat *b)
{
  GLfloat dstc[3];
  
  dstc[0] = a[1] * b[2] - a[2] * b[1];
  dstc[1] = a[2] * b[0] - a[0] * b[2];
  dstc[2] = a[0] * b[1] - a[1] * b[0];
  dst[0] = dstc[0]; dst[1] = dstc[1]; dst[2] = dstc[2];
}

static GLfloat
dot (GLfloat *a, GLfloat *b)
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static void
scale (GLfloat *dst, GLfloat *src, GLfloat scale)
{
  dst[0] = src[0] * scale;
  dst[1] = src[1] * scale;
  dst[2] = src[2] * scale;
}

static void
add (GLfloat *dst, GLfloat *a, GLfloat *b)
{
  dst[0] = a[0] + b[0];
  dst[1] = a[1] + b[1];
  dst[2] = a[2] + b[2];
}

static void
sub (GLfloat *dst, GLfloat *a, GLfloat *b)
{
  dst[0] = a[0] - b[0];
  dst[1] = a[1] - b[1];
  dst[2] = a[2] - b[2];
}

static matrix_t objmatrix __attribute__((aligned(32)));

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  float rot1 = 0.0, rot2 = 0.0, rot3 = 0.0;
  kos_img_t cubetxr;
  GLuint texture[2];
  pvr_ptr_t texaddr;
  pvr_ptr_t bumpmap;
  /* Consider this as the vector from the object to the light, for now.  */
  GLfloat light_position[3] = { 0.7, 0.7, 2.0 };
  
  normalize (&light_position[0]);
  
  cable_type = vid_check_cable ();
  
  printf ("KOS says M_PI is: %f\n", M_PI);
  
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);
  
  init_pvr ();
  
  png_to_img ("/rd/cube.png", PNG_NO_ALPHA, &cubetxr);
  
  texaddr = pvr_mem_malloc (cubetxr.w * cubetxr.h * 2);
  pvr_txr_load_kimg (&cubetxr, texaddr, PVR_TXRLOAD_INVERT_Y);
  kos_img_free (&cubetxr, 0);
  
  bumpmap = load_bumpmap ("/rd/bump.raw");
  
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
  gluLookAt (0.0,   0.0,  -4.5,		/* Eye position.  */
	     0.0,   0.0,   0.0,		/* Centre.  */
	     0.0,   1.0,   0.0);	/* Up.  */

  glGenTextures (2, &texture[0]);

  /* Ordinary texture.  */
  glBindTexture (GL_TEXTURE_2D, texture[0]);
  glKosTex2D (GL_RGB565_TWID, cubetxr.w, cubetxr.h, texaddr);

  /* Bump texture.  */
  glBindTexture (GL_TEXTURE_2D, texture[1]);

  /*pvr_poly_cxt_txr (cxt, PVR_LIST_OP_POLY,
                    PVR_TXRFMT_BUMP | PVR_TXRFMT_TWIDDLED,
                    128, 128, bumpmap, PVR_FILTER_BILINEAR);*/
  glKosTex2D (GL_BUMP_TWID, 128, 128, bumpmap);

  glTexEnvi (GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL);

  /* Break the nice abstraction.  Tweak for bump mapping.  */
  /*cxt = (pvr_poly_cxt_t *) texture[1];
  cxt->gen.specular = PVR_SPECULAR_ENABLE;*/
  /*pvr_poly_compile (&bumphdr, cxt);*/
  
  printf ("objmatrix at %p\n", objmatrix);
  
  while (!quit)
    {
      int faces;
      GLfloat transformed_normals
        [sizeof (face_normals) / (3 * sizeof (GLfloat))][3];
      GLfloat transformed_orient
        [sizeof (face_orient) / (3 * sizeof (GLfloat))][3];
      
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()
      
      glKosBeginFrame ();
      
      glPushMatrix ();
      
      glRotatef (rot1, 1.0, 0.0, 0.0);
      glRotatef (rot2, 0.0, 1.0, 0.0);
      glRotatef (rot3, 0.0, 0.0, 1.0);
      
      rot1 += 0.1;
      rot2 += 0.2;
      rot3 += 0.3;

      /* Get the object's transformation matrix.  */
      glGetFloatv (GL_MODELVIEW_MATRIX, &objmatrix[0][0]);
      /* We care only about rotation for now.  */
      objmatrix[0][3] = objmatrix[1][3] = objmatrix[2][3] = 0.0;
      objmatrix[3][0] = objmatrix[3][1] = objmatrix[3][2] = 0.0;
      objmatrix[3][3] = 1.0;

      /*printf ("Got matrix:\n");
      printf ("[ %f %f %f %f ]\n", objmatrix[0][0], objmatrix[0][1],
				   objmatrix[0][2], objmatrix[0][3]);
      printf ("[ %f %f %f %f ]\n", objmatrix[1][0], objmatrix[1][1],
				   objmatrix[1][2], objmatrix[1][3]);
      printf ("[ %f %f %f %f ]\n", objmatrix[2][0], objmatrix[2][1],
				   objmatrix[2][2], objmatrix[2][3]);
      printf ("[ %f %f %f %f ]\n", objmatrix[3][0], objmatrix[3][1],
				   objmatrix[3][2], objmatrix[3][3]);*/

      glBindTexture (GL_TEXTURE_2D, texture[1]);
      glEnable (GL_KOS_OFFSET_COLOR);
      
      /* Do these all in one go.  */
      mat_load ((matrix_t *) &objmatrix[0][0]);

      /* Note: mat_transform is only for 3D->2D (perspective)
         transformations!  This won't be quite as quick, most likely.  */
      for (faces = 0; faces < 6; faces++)
        {
	  GLfloat x = face_normals[faces][0];
	  GLfloat y = face_normals[faces][1];
	  GLfloat z = face_normals[faces][2];
	  GLfloat w = 1.0;
	  mat_trans_nodiv (x, y, z, w);
	  transformed_normals[faces][0] = x;
	  transformed_normals[faces][1] = y;
	  transformed_normals[faces][2] = z;

	  x = face_orient[faces][0];
	  y = face_orient[faces][1];
	  z = face_orient[faces][2];
	  w = 1.0;
	  mat_trans_nodiv (x, y, z, w);
	  transformed_orient[faces][0] = x;
	  transformed_orient[faces][1] = y;
	  transformed_orient[faces][2] = z;
	}
      glKosMatrixDirty ();
      
      for (faces = 0; faces < 6; faces++)
        {
	  int strip, over_pi, over_2pi;
	  GLfloat s_dot_n, f[3], d[3], t, q;
	  GLfloat d_cross_r[3], dxr_len, d_len;

	  s_dot_n = dot (&transformed_normals[faces][0], &light_position[0]);
	  /* Elevation (T) angle:
	     s.n = |s| |n| cos T
	     T = acos (s.n / (|s| * |n|))
	     |s| and |n| are both 1.  */
	  t = M_PI / 2 - acosf (s_dot_n);
	  
	  if (t < 0)
	    t = 0;
	  
	  /* Rotation (Q) angle:
	     d x r = (|d| |r| sin Q) n
	     |d x r| / (|d| |r|) = sin Q.  */
	  scale (&f[0], &transformed_normals[faces][0], s_dot_n);
	  sub (&d[0], &light_position[0], &f[0]);
	  cross (&d_cross_r[0], &d[0], &transformed_orient[faces][0]);
	  dxr_len = length (&d_cross_r[0]);
	  d_len = length (&d[0]);
	  q = asinf (dxr_len / d_len);
	  
	  over_pi = dot (&d[0], &transformed_orient[faces][0]) < 0;
	  if (over_pi)
	    q = M_PI - q;

	  over_2pi = dot (&d_cross_r[0], &transformed_normals[faces][0]) < 0;
	  if (over_2pi)
	    q = 2 * M_PI - q;

	  /*printf ("length of n: %f\n",
		 length (&transformed_normals[faces][0]));
	  printf ("%d: [ %f %f %f ]\n", faces, transformed_normals[faces][0],
	    transformed_normals[faces][1], transformed_normals[faces][2]);

	  printf ("length of r: %f\n", length (&transformed_orient[faces][0]));
	  printf ("%d: [ %f %f %f ]\n", faces, transformed_orient[faces][0],
	    transformed_orient[faces][1], transformed_orient[faces][2]);*/

	  glBegin (GL_TRIANGLE_STRIP);

	  set_bump_direction (t, 2 * M_PI - q, 0.7);
	  glColor4ub (0, 0, 0, 0);

	  for (strip = 0; strip < 4; strip++)
	    {
	      glTexCoord2fv (texcoords[strip]);
	      glVertex3fv (points[tristrips[faces][strip]]);
	    }

	  glEnd ();
	}

      glDisable (GL_KOS_OFFSET_COLOR);
      /* Finish opaque polygon list, start translucent polygon list.  */
     /* glKosFinishList ();*/

      /* Finish translucent polygon list, start punchthru list.  */
      glKosFinishList ();

      glBindTexture (GL_TEXTURE_2D, texture[0]);
      glBlendFunc (GL_DST_COLOR, GL_ZERO);
      
      for (faces = 0; faces < 6; faces++)
        {
	  int strip;
	  	  
	  glBegin (GL_TRIANGLE_STRIP);

	  glColor4ub (colour[faces][0], colour[faces][1], colour[faces][2],
		      colour[faces][3]);

	  for (strip = 0; strip < 4; strip++)
	    {
	      glTexCoord2fv (texcoords[strip]);
	      glVertex3fv (points[tristrips[faces][strip]]);
	    }
	  
	  glEnd ();
	}

      glPopMatrix ();
                  
      glKosFinishFrame ();
    }

  glKosShutdown ();
  
  pvr_shutdown ();
  
  vid_shutdown ();
  
  return 0;
}
