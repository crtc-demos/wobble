/* Fake phong highlighting.  */

#include <math.h>
#include <stdlib.h>
#include <alloca.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>

#include "geosphere.h"
#include "vector.h"
#include "restrip.h"

#define PARAMETERISATION 1

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

float light_pos[] = {0.5, 0.5, -3.0};
float eye_pos[] = {0.0, 0.0, -4.5};

static matrix_t camera __attribute__((aligned(32)));
static matrix_t invcamera __attribute__((aligned(32)));

pvr_ptr_t highlight = 0;

static void
render_highlight (float hardness)
{
  uint8 *tmphilight;
  unsigned int x, y;
  float norm[3] = {0.0, 0.0, 1.0};
  
  tmphilight = malloc (256 * 256);
  
  /* We need a way to map 2D texture coordinates to points on a unit hemisphere,
     such that interpolating linearly along the texture approximates travelling
     smoothly along a great circle around the hemisphere.  Not sure how best to
     do this.  Maybe a parabolic texture?
     First try spherical polar coordinates.  These will be all bunched together
     at the top and bottom, but there'll be nothing interesting in those regions
     for our highlight anyway.  */
  
  /***
        <----->  P (around)
      .. - |-__..
     '  -=###=-\ `
    '	   |_/  | '
   '	  T|/   |  '
  -|-------+----+--|--X
   '	   |    |  '
   ' 	   |(arc)  '
    '  	   |  _/  ,
      ' ...|--  '    \ 
           Y          Z (+Z towards viewer)
  
  ***/

#if PARAMETERISATION == 0
  for (y = 0; y < 256; y++)
    for (x = 0; x < 256; x++)
      {
        float t = ((float) y / 255.0) * M_PI;
	float p = ((float) (x - 127.5) / 127.5) * M_PI / 2.0;
	float rad;
	float reflection[3];
	float dot;
	
	reflection[1] = fcos (t);
	rad = fsin (t);
	reflection[2] = fcos (p) * rad;
	reflection[0] = fsin (p) * rad;
	
	dot = vec_dot (reflection, norm);
	
	tmphilight[y * 256 + x] = 255.0 * powf (dot, hardness);
      }
#elif PARAMETERISATION == 1
  for (y = 0; y < 256; y++)
    for (x = 0; x < 256; x++)
      {
        float reflection[3];
	float dot;
	
	reflection[0] = (x - 127.5) / 127.5;
	reflection[1] = (y - 127.5) / 127.5;
	reflection[2] = fsqrt (1.0 - reflection[0] * reflection[0]
			       - reflection[1] * reflection[1]);
	
	dot = vec_dot (reflection, norm);
	
	tmphilight[y * 256 + x] = 255.0 * powf (dot, hardness);
      }
#endif
  
  if (highlight == 0)
    highlight = pvr_mem_malloc (256 * 256);

  pvr_txr_load_ex (tmphilight, highlight, 256, 256, PVR_TXRLOAD_8BPP);
  
  free (tmphilight);
}

static void
set_grey_palette (void)
{
  unsigned int i;
  
  pvr_set_pal_format (PVR_PAL_ARGB8888);
  
  for (i = 0; i < 256; i++)
    {
      unsigned int palentry;
      
      /* Grey ramp with solid alpha.  */
      palentry = 0xff000000 | (i << 16) | (i << 8) | i;
      pvr_set_pal_entry (i, palentry);
    }
}

#if 0
static void
lightsource_vertex (float *vertex, float *norm)
{
  float norm_light[3];
  float out;
  float eye_to_vertex[3], tmp[3], reflection[3];
  float eye_dot_norm, reflect_dot_light;
  int r, g, b;
  
  vec_normalize (norm_light, light_pos);
  
  vec_sub (eye_to_vertex, eye_pos, vertex);
  vec_normalize (eye_to_vertex, eye_to_vertex);
  
  eye_dot_norm = vec_dot (eye_to_vertex, norm);
  vec_scale (tmp, norm, 2.0 * eye_dot_norm);
  vec_sub (reflection, tmp, eye_to_vertex);
  reflect_dot_light = vec_dot (reflection, norm_light);
  
  if (reflect_dot_light > 0)
    reflect_dot_light = powf (reflect_dot_light, 50.0f);
  else
    reflect_dot_light = 0;
  
  out = vec_dot (norm_light, norm);
  
  if (out < 0)
    out = 0;

  r = 64 + out * 64;
  g = out * 64;
  b = out * 64;
  
  r += (255 - r) * reflect_dot_light;
  g += (255 - g) * reflect_dot_light;
  b += (255 - b) * reflect_dot_light;

  glColor4ub (r, g, b, 255);
}
#endif

/* LIGHT is the direction of the light from the vertex in question, and
   LIGHT_UPDIR is an "upwards" orientation vector for the light.  It doesn't
   have to be exactly perpendicular to the light vector, but it should be
   approximately so, as well as consistent for a given object.
   LIGHT, REFLECTION, LIGHT_UPDIR should be normalized before calling.  */

static int
fake_phong_texcoords (float *u, float *v, float reflection[3], float light[3],
		      float light_updir[3], float *transformed_z)
{
#if PARAMETERISATION == 0
  float sideways[3], tmp[3];
  float light_planar[3], reflection_planar[3];
  float yaw, pitch;

  vec_cross (sideways, light, light_updir);
  vec_normalize (sideways, sideways);
  
  /* | A x B | = |A| |B| sin T.  */
  
  /* First, drop light and reflection vectors onto the plane with normal
     "light_updir".  */

  vec_scale (tmp, light_updir, vec_dot (light, light_updir));
  vec_sub (light_planar, light, tmp);
  
  vec_scale (tmp, light_updir, vec_dot (reflection, light_updir));
  vec_sub (reflection_planar, reflection, tmp);
  
  yaw = vec_angle (light_planar, reflection_planar, NULL) / 2;
  
  pitch = (vec_angle (light_updir, light, NULL)
           - vec_angle (light_updir, reflection, NULL)) / 2;
  
  if (yaw > -M_PI / 2 && yaw < M_PI / 2
      && pitch > -M_PI / 2 && pitch < M_PI / 2)
    {
      *u = 0.5 + yaw / M_PI;
      *v = 0.5 + pitch / M_PI;
      return 1;
    }
  else
    {
      *u = 0.0;
      *v = 0.0;
      return 0;
    }
#elif PARAMETERISATION == 1
  float light_sideways[3], light_realup[3];
  matrix_t light_basis;
  float reflection4[4], x_reflection[4];
  
  vec_cross (light_sideways, light, light_updir);
  vec_cross (light_realup, light_sideways, light);
  
  light_basis[0][0] = light_sideways[0];
  light_basis[1][0] = light_sideways[1];
  light_basis[2][0] = light_sideways[2];
  light_basis[3][0] = 0.0;
  
  light_basis[0][1] = light_realup[0];
  light_basis[1][1] = light_realup[1];
  light_basis[2][1] = light_realup[2];
  light_basis[3][1] = 0.0;
  
  light_basis[0][2] = light[0];
  light_basis[1][2] = light[1];
  light_basis[2][2] = light[2];
  light_basis[3][2] = 0.0;
  
  light_basis[0][3] = 0.0;
  light_basis[1][3] = 0.0;
  light_basis[2][3] = 0.0;
  light_basis[3][3] = 1.0;
  
  memcpy (reflection4, reflection, sizeof (float) * 3);
  reflection4[3] = 1.0;
  
  vec_transform (x_reflection, (float *) light_basis, reflection4);

  *transformed_z = x_reflection[2];

  *u = x_reflection[0] * 0.5 + 0.5;
  *v = x_reflection[1] * 0.5 + 0.5;

  return 1;
#endif
}

float ambient_red = 64;
float ambient_green = 0;
float ambient_blue = 0;
float pigment_red = 128;
float pigment_green = 0;
float pigment_blue = 0;

static int
lightsource_fake_phong (float *u_p, float *v_p, float *vertex, float *norm,
			int pass, float *transformed_z)
{
  float norm_light[3];
  float out;
  float eye_to_vertex[3], tmp[3], reflection[3];
  float eye_dot_norm;
  int r, g, b;
  
  vec_normalize (norm_light, light_pos);
  
  vec_sub (eye_to_vertex, eye_pos, vertex);
  vec_normalize (eye_to_vertex, eye_to_vertex);
  
  eye_dot_norm = vec_dot (eye_to_vertex, norm);
  vec_scale (tmp, norm, 2.0 * eye_dot_norm);
  vec_sub (reflection, tmp, eye_to_vertex);
    
  if (pass == 0)
    {
      out = vec_dot (norm_light, norm);

      if (out < 0)
	out = 0;

      r = ambient_red + out * pigment_red;
      g = ambient_green + out * pigment_green;
      b = ambient_blue + out * pigment_blue;

      glColor4ub (r, g, b, 255);
      
      return 1;
    }
  else
    {
      float light_up[3] = {0.0, 1.0, 0.0};
      
      return fake_phong_texcoords (u_p, v_p, reflection, norm_light, light_up,
				   transformed_z);
    }
}

static strip *stripbuf = NULL;
static unsigned int capacity = 0;

typedef struct
{
  float texc[2];
  float transformed_z;
} vertex_info;

static int
classify_triangle (float tri[3][3], int clockwise, void *extra)
{
  vertex_info *v_inf = extra;

  if (v_inf[0].transformed_z >= 0 || v_inf[1].transformed_z >= 0
      || v_inf[2].transformed_z >= 0)
    return 0;
  else
    return -1;
}

void
render_geosphere (strip *strips, int pass)
{
  int sidx;
  strip *strip_starts[1] = { 0 };
  strip *strip_ends[1] = { 0 };
  strip *strip_iter, *to_draw = strips;
  
  if (capacity == 0)
    {
      capacity = 10;
      stripbuf = malloc (sizeof (strip) * capacity);
    }
    
  //srand (0);
  
  if (pass == 1)
    {
      glColor4ub (255, 255, 255, 255);

      for (strip_iter = strips; strip_iter; strip_iter = strip_iter->next)
	{
	  unsigned int strip_length = strip_iter->length;
	  float (*data)[][3] = strip_iter->start;
	  vertex_info *v_inf;
	  
	  v_inf = alloca (sizeof (vertex_info) * strip_length);

	  for (sidx = 0; sidx < strip_length; sidx++)
            {
	      float u, v;
	      lightsource_fake_phong (&u, &v, (float *) &(*data)[sidx],
				      (float *) &(*data)[sidx], pass,
				      &v_inf[sidx].transformed_z);

	      v_inf[sidx].texc[0] = u;
	      v_inf[sidx].texc[1] = v;
	    }

	  strip_iter->extra = v_inf;
	  strip_iter->extra_elsize = sizeof (vertex_info);
	}

      restrip_list (strips, classify_triangle, strip_starts, strip_ends,
		    &stripbuf, &capacity);
      to_draw = strip_starts[0];
    }

  for (strip_iter = to_draw; strip_iter; strip_iter = strip_iter->next)
    {
      int strip_length = strip_iter->length;
      float (*data)[][3] = strip_iter->start;
      vertex_info *v_inf = strip_iter->extra;
      int invert = 0;

      if (strip_length < 0)
        {
          strip_length = -strip_length;
	  invert = 1;
	}

#if 0
      if (pass == 1 && (rand () & 31) < 16)
        continue;
#endif

      glBegin (GL_TRIANGLE_STRIP);
      
      for (sidx = 0; sidx < strip_length; sidx++)
        {
	  if (pass == 0)
	    lightsource_fake_phong (0, 0, (float *) &(*data)[sidx],
				    (float *) &(*data)[sidx], pass, NULL);
          else
	    glTexCoord2f (v_inf[sidx].texc[0], v_inf[sidx].texc[1]);

	  if (invert == 1 && sidx == 0)
	    glVertex3fv ((GLfloat *) &(*data)[0]);

	  glVertex3fv ((GLfloat *) &(*data)[sidx]);
	}
      
      glEnd ();
    }

#if 0
  if (pass == 1)
    for (strip_iter = strips; strip_iter; strip_iter = strip_iter->next)
      free (strip_iter->extra);
#endif
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
    1			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

float rot1 = 0.0;
float rot2 = 0.0;

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  float *spheredata;
  strip *spherestrips;
  int strips, strip_length;
  GLuint texture[1];
  
  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);
  
  dbgio_printf ("Generating sphere data... ");
  spheredata = make_geosphere (2, &strips, &strip_length);
  spherestrips = strips_for_geosphere (spheredata, strips, strip_length);
  dbgio_printf ("done. %d strips of length %d\n", strips, strip_length);

#if 0  
  dbgio_printf ("sphere data:\n");
  for (j = 0; j < strips; j++)
    {
      dbgio_printf ("strip %d:\n", j);
      for (i = 0; i < strip_length; i++)
        {
	  int idx = j * strip_length + i;
          dbgio_printf ("[ %f %f %f ]\n", (double) spheredata[idx * 3],
					  (double) spheredata[idx * 3 + 1],
					  (double) spheredata[idx * 3 + 2]);
	}
      dbgio_printf ("\n");
    }
#endif
  
  init_pvr ();
  glKosInit ();
  
  glViewport (0, 0, 1280, 480);
  
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

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  gluLookAt (0.0,  0.0, -4.5,		/* Eye position.  */
	     0.0,  0.0,  0.0,		/* Centre.  */
	     0.0,  1.0,  0.0);		/* Up.  */
  
  glGetFloatv (GL_MODELVIEW_MATRIX, &camera[0][0]);
  vec_transpose_rotation (&invcamera[0][0], &camera[0][0]);
  
  glGenTextures (1, &texture[0]);

  render_highlight (30.0f);
  glBindTexture (GL_TEXTURE_2D, texture[0]);
  glKosTex2D (PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_TWIDDLED, 256, 256, highlight);
  
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  set_grey_palette ();
    
  glBlendFunc (GL_ONE, GL_ONE);
  
  while (!quit)
    {
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()
      
      glKosBeginFrame ();
      
#if 0
      glBegin (GL_QUADS);
      glColor4ub (255, 255, 255, 255);
      
      glTexCoord2f (0.0, 0.0);
      glVertex3f (0.0, 0.0, -2.0);
      glTexCoord2f (0.0, 1.0);
      glVertex3f (0.0, 0.5, -2.0);
      glTexCoord2f (1.0, 1.0);
      glVertex3f (0.5, 0.5, -2.0);
      glTexCoord2f (1.0, 0.0);
      glVertex3f (0.5, 0.0, -2.0);
      glEnd ();
#endif
      
      /*glPushMatrix ();
      glRotatef (rot1, 0.0, 1.0, 0.0);*/
      
      light_pos[0] = cosf (rot1) * 10;
      light_pos[2] = sinf (rot1) * 10;
      light_pos[1] = 2.0f;
      
      rot1 += 0.05;
      if (rot1 > 2 * M_PI)
        rot1 -= 2 * M_PI;
      
      rot2 += 0.18;
      if (rot2 > 2 * M_PI)
        rot2 -= 2 * M_PI;
      
      ambient_red = 64;
      ambient_green = 0;
      ambient_blue = 0;
      pigment_red = 128;
      pigment_green = 0;
      pigment_blue = 0;

      glDisable (GL_TEXTURE_2D);
      glPushMatrix ();
      glScalef (0.8 + 0.2 * sinf (rot2 + 0.5),
		0.8 + 0.2 * sinf (rot2 + 0.5),
		0.8 + 0.2 * sinf (rot2 + 0.5));
      render_geosphere (spherestrips, 0);
      glPopMatrix ();
      
      glPushMatrix ();
      glScalef (0.8 + 0.2 * sinf (rot2 + 1.0),
		0.8 + 0.2 * sinf (rot2 + 1.0),
		0.8 + 0.2 * sinf (rot2 + 1.0));
      glTranslatef (0.0, 1.0, 0.0);
      ambient_red = 0;
      ambient_green = 64;
      ambient_blue = 0;
      pigment_red = 0;
      pigment_green = 128;
      pigment_blue = 0;
      render_geosphere (spherestrips, 0);
      glPopMatrix ();

      glPushMatrix ();
      glScalef (0.8 + 0.2 * sinf (rot2),
		0.8 + 0.2 * sinf (rot2),
		0.8 + 0.2 * sinf (rot2));
      glTranslatef (0.0, -1.0, 0.0);
      ambient_red = 0;
      ambient_green = 0;
      ambient_blue = 64;
      pigment_red = 0;
      pigment_green = 0;
      pigment_blue = 128;
      render_geosphere (spherestrips, 0);
      glPopMatrix ();

      /*glPopMatrix ();*/
      
      glKosFinishList ();
      
      glEnable (GL_TEXTURE_2D);
      glPushMatrix ();
      glScalef (0.8 + 0.2 * sinf (rot2 + 0.5),
		0.8 + 0.2 * sinf (rot2 + 0.5),
		0.8 + 0.2 * sinf (rot2 + 0.5));
      render_geosphere (spherestrips, 1);
      glPopMatrix ();

      glPushMatrix ();
      glScalef (0.8 + 0.2 * sinf (rot2 + 1.0),
		0.8 + 0.2 * sinf (rot2 + 1.0),
		0.8 + 0.2 * sinf (rot2 + 1.0));
      glTranslatef (0.0, 1.0, 0.0);
      render_geosphere (spherestrips, 1);
      glPopMatrix ();

      glPushMatrix ();
      glScalef (0.8 + 0.2 * sinf (rot2),
		0.8 + 0.2 * sinf (rot2),
		0.8 + 0.2 * sinf (rot2));
      glTranslatef (0.0, -1.0, 0.0);
      render_geosphere (spherestrips, 1);
      glPopMatrix ();

      glKosFinishList ();
      
      glKosFinishFrame ();
    }

  glKosShutdown ();
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
