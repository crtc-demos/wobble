/* A wobbly tube.  */

#include <math.h>
#include <stdlib.h>
#include <alloca.h>
#include <stdint.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>
#include <kmg/kmg.h>
#include <jpeg/jpeg.h>

#define DC_FAST_MATHS 1
#include <dc/fmath.h>

#include "vector.h"
#include "restrip.h"
#include "fakephong.h"
#include "palette.h"
#include "object.h"
#include "loader.h"
#include "tube.h"
#include "skybox.h"
#include "timing.h"
#include "cam_path.h"

static float c_path[][3] = {
{ -0.586424, -1.044915, -11.418626 }, /* eye pos */       
{ -2.448233, -0.944130, -9.068411 }, /* lookat */         

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -2.825508, -1.133100, -9.423950 }, /* lookat */  

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -3.393301, -1.437040, -10.434916 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -3.502799, -1.213985, -10.735853 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -3.578385, -1.023326, -11.637036 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -3.372299, -0.761229, -12.494926 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -2.906713, -0.569886, -13.259986 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -2.319664, -0.790428, -13.854014 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -1.463375, -0.946000, -14.285885 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -0.596011, -1.137491, -14.417182 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -0.100188, -1.344176, -14.363794 }, /* lookat */ 

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 0.713668, -1.303966, -14.109843 }, /* lookat */  

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 1.005487, -1.146833, -13.959379 }, /* lookat */  

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 1.650318, -0.803474, -13.403239 }, /* lookat */  

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 2.102892, -0.678835, -12.696733 }, /* lookat */  

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 2.333492, -0.605474, -11.948704 }, /* lookat */  

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 2.320197, -0.609449, -10.817026 }, /* lookat */  

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 1.852802, -0.492290, -9.761888 }, /* lookat */   

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 0.967272, -0.992442, -8.852837 }, /* lookat */   

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ 0.333732, -1.095392, -8.563671 }, /* lookat */   

{ -0.586424, -1.044915, -11.418626 }, /* eye pos */
{ -0.159999, -1.147650, -8.450865 }, /* lookat */  

{ 1.301188, -0.334455, -7.686276 }, /* eye pos */
{ 1.166137, -0.633346, -4.704259 }, /* lookat */ 

{ 3.395462, -0.334455, -7.550888 }, /* eye pos */
{ 2.692583, -0.772885, -4.667532 }, /* lookat */ 

{ 6.043315, -0.334455, -6.643354 }, /* eye pos */
{ 5.047009, -0.753370, -3.844804 }, /* lookat */ 

{ 4.152414, -1.026477, -2.167651 }, /* eye pos */
{ 2.590984, -1.450007, 0.358722 }, /* lookat */  

{ 1.657212, -2.056083, -3.272781 }, /* eye pos */
{ 1.718066, -2.162108, -0.275273 }, /* lookat */ 

{ 1.952226, -4.717242, -7.135271 }, /* eye pos */
{ 0.714258, -4.583164, -4.405901 }, /* lookat */ 

{ -8.045043, -4.717242, -7.102343 }, /* eye pos */
{ -6.927948, -4.009868, -4.409441 }, /* lookat */ 

{ -11.454959, -4.717242, -5.066177 }, /* eye pos */
{ -9.226446, -4.326194, -3.096199 }, /* lookat */  

{ -1.760910, -1.526635, -5.594282 }, /* eye pos */
{ -0.884758, -1.129397, -2.752705 }, /* lookat */ 

{ 0.263818, -1.012637, -2.030194 }, /* eye pos */
{ 0.453614, -0.631454, 0.939432 }, /* lookat */  

{ 2.107844, -1.012637, -2.021813 }, /* eye pos */
{ 1.168214, -0.482991, 0.777574 }, /* lookat */  

{ 3.021059, -0.628360, 0.506253 }, /* eye pos */
{ 1.560683, -0.081028, 3.069011 }, /* lookat */ 

{ 11.036502, -2.011390, -2.009330 }, /* eye pos */
{ 8.517568, -1.573075, -0.439983 }, /* lookat */  

{ 18.162079, 0.822587, -0.065040 }, /* eye pos */
{ 15.257492, 0.447797, 0.585272 }, /* lookat */  

{ 7.691171, 0.522893, 0.192884 }, /* eye pos */
{ 5.004866, 0.219993, 1.493658 }, /* lookat */ 

{ 4.427484, 0.012193, 1.893288 }, /* eye pos */
{ 1.920148, -0.529130, 3.448999 }, /* lookat */

{ 2.704463, 0.012193, -0.223215 }, /* eye pos */
{ 1.096435, -0.253829, 2.295411 }, /* lookat */ 

{ 3.998953, 0.188566, -2.487372 }, /* eye pos */
{ 3.063522, 0.253141, 0.362329 }, /* lookat */  

{ 5.718472, -0.083371, -11.376268 }, /* eye pos */
{ 5.204401, -0.048230, -8.420850 }, /* lookat */  

{ 6.037628, -0.164927, -16.469408 }, /* eye pos */
{ 5.945291, -0.113717, -13.471267 }, /* lookat */ 

{ 4.779523, -0.425068, -8.387170 }, /* eye pos */
{ 4.118527, -0.484830, -5.461505 }, /* lookat */ 

{ 3.310572, -0.602802, -3.769159 }, /* eye pos */
{ 2.295870, -0.731512, -0.948908 }, /* lookat */ 

{ 2.076441, -0.754992, -0.871697 }, /* eye pos */
{ 0.688195, -0.882472, 1.784715 }, /* lookat */  

{ 3.976639, -0.594819, -4.573738 }, /* eye pos */
{ 2.533521, -0.701590, -1.945809 }, /* lookat */ 

{ 6.599042, -0.507847, -6.134903 }, /* eye pos */
{ 5.004484, -0.659437, -3.598291 }, /* lookat */ 

{ 5.381870, -0.507847, -6.820310 }, /* eye pos */
{ 4.309932, -0.578812, -4.019254 }, /* lookat */ 

{ 3.194678, -0.507847, -7.507901 }, /* eye pos */
{ 2.703886, -0.487918, -4.548387 }, /* lookat */ 

{ 2.553435, -0.507847, -7.614243 }, /* eye pos */
{ 1.684192, -0.509208, -4.742934 }, /* lookat */ 

{ 0.588588, -0.507847, -7.888205 }, /* eye pos */
{ 0.667671, -0.547680, -4.889512 }, /* lookat */ 

{ -2.455803, -0.507847, -7.399350 }, /* eye pos */
{ -1.401586, -0.295454, -4.598722 }, /* lookat */ 

{ -0.683882, -0.146868, -3.167045 }, /* eye pos */
{ 0.671400, 0.132745, -0.505274 }, /* lookat */   

{ 1.108133, -0.146868, -3.076772 }, /* eye pos */
{ 0.302297, 0.132132, -0.200527 }, /* lookat */  

{ 2.344945, -0.882895, -7.611675 }, /* eye pos */
{ 2.034727, -0.275223, -4.690288 }, /* lookat */ 

{ 1.835282, -2.471832, -14.420276 }, /* eye pos */
{ 2.206274, -2.186734, -11.456986 }, /* lookat */ 

{ 2.898737, -2.327355, -5.100996 }, /* eye pos */
{ 2.297474, -2.522254, -2.168335 }, /* lookat */ 

{ 5.768981, -2.294265, -3.888674 }, /* eye pos */
{ 4.046059, -2.613952, -1.453650 }, /* lookat */ 

{ -1.665549, -1.986162, -9.005747 }, /* eye pos */
{ -0.039956, -2.076977, -6.485985 }, /* lookat */ 

{ -0.858639, -2.330424, -5.401360 }, /* eye pos */
{ -0.392736, -2.598733, -2.449929 }, /* lookat */ 

{ 2.107198, -2.484426, -2.449427 }, /* eye pos */
{ 1.653862, -2.625640, 0.512759 }, /* lookat */  

{ 5.648686, -4.364491, -13.035640 }, /* eye pos */
{ 4.354966, -3.893338, -10.370250 }, /* lookat */ 

{ 3.356826, -2.905280, -6.783840 }, /* eye pos */
{ 2.553658, -2.161092, -3.990794 }, /* lookat */

{ 1.544362, -2.905280, -7.461915 }, /* eye pos */
{ 1.026650, -2.170352, -4.599772 }, /* lookat */

{ 0.109107, -2.905280, -7.412368 }, /* eye pos */
{ 0.314999, -2.080705, -4.535272 }, /* lookat */

{ -6.109624, -7.151562, -2.411847 }, /* eye pos */
{ -4.056755, -5.587576, -0.882247 }, /* lookat */

{ -4.358788, -9.962052, -1.153594 }, /* eye pos */
{ -2.735308, -7.667470, -0.105164 }, /* lookat */

{ -7.767320, -7.379618, -7.568145 }, /* eye pos */
{ -6.034231, -6.363174, -5.340312 }, /* lookat */

{ -4.957791, -6.076079, -10.562127 }, /* eye pos */
{ -3.807211, -5.209943, -7.930402 }, /* lookat */

{ 0.916504, -0.213500, -13.007551 }, /* eye pos */        
{ 0.448063, -0.141159, -10.045233 }, /* lookat */         

{ 0.475549, -0.154646, -10.635747 }, /* eye pos */
{ -0.156297, -0.081748, -7.703946 }, /* lookat */ 

{ -0.326891, -0.053417, -6.693218 }, /* eye pos */
{ -0.328287, 0.058983, -3.695324 }, /* lookat */  

{ -2.526312, -0.041688, -5.330235 }, /* eye pos */
{ -1.874333, -0.101572, -2.402551 }, /* lookat */ 

{ -3.266421, -0.068986, -2.834437 }, /* eye pos */
{ -2.262975, -0.106425, -0.007478 }, /* lookat */ 

{ -4.627778, -1.172114, 0.695675 }, /* eye pos */
{ -2.472018, -1.089903, 2.780367 }, /* lookat */ 

{ -5.944698, -1.172114, 2.316474 }, /* eye pos */
{ -3.218229, -1.218481, 3.567161 }, /* lookat */ 

{ -6.396526, -1.172114, 3.995448 }, /* eye pos */
{ -3.419277, -1.108958, 4.358767 }, /* lookat */ 

{ -11.891425, -0.949504, 8.686245 }, /* eye pos */
{ -9.274755, -1.111028, 7.227838 }, /* lookat */  

{ -9.568103, -1.448779, 11.648743 }, /* eye pos */
{ -7.445002, -1.560533, 9.532152 }, /* lookat */  

{ -7.557625, -1.425868, 14.422613 }, /* eye pos */
{ -5.851807, -1.474317, 11.955259 }, /* lookat */ 

{ -5.645427, -1.430617, 16.300415 }, /* eye pos */
{ -4.866322, -1.234913, 13.409966 }, /* lookat */ 

{ -1.254217, -1.491228, 17.814949 }, /* eye pos */
{ -1.486972, -1.051417, 14.856505 }, /* lookat */ 

{ 3.718792, -1.544846, 16.322844 }, /* eye pos */
{ 2.460259, -0.877866, 13.682535 }, /* lookat */ 

{ 5.917395, -0.578131, 11.851124 }, /* eye pos */
{ 4.173225, 0.190025, 9.534270 }, /* lookat */   

{ 11.893769, -1.656916, 10.699069 }, /* eye pos */
{ 9.248360, -0.558254, 9.807583 }, /* lookat */   

{ 19.892738, -1.726446, 7.152452 }, /* eye pos */
{ 16.918404, -1.510509, 7.479100 }, /* lookat */ 

{ 18.621225, -1.728749, 1.820345 }, /* eye pos */
{ 15.780590, -1.534266, 2.765316 }, /* lookat */ 

{ 16.204836, -1.728749, -3.274595 }, /* eye pos */
{ 13.410924, -1.457734, -2.216007 }, /* lookat */ 

{ 11.240480, -1.534019, -7.112754 }, /* eye pos */
{ 9.053793, -1.270899, -5.075804 }, /* lookat */  

{ 7.732173, -1.414063, -9.938591 }, /* eye pos */
{ 6.509355, -1.395921, -7.199178 }, /* lookat */ 

{ 5.888961, -1.416087, -10.951210 }, /* eye pos */
{ 5.318398, -1.334200, -8.007105 }, /* lookat */  

{ 3.087669, -1.351568, -9.804041 }, /* eye pos */
{ 3.038728, -1.213449, -6.807622 }, /* lookat */ 

{ 0.852031, -1.351568, -9.609607 }, /* eye pos */
{ 1.398839, -1.094308, -6.671101 }, /* lookat */ 

{ 1.222629, -1.171169, -7.575371 }, /* eye pos */
{ 1.609093, -0.866990, -4.615958 }, /* lookat */ 

{ 1.354464, -0.672661, -3.794102 }, /* eye pos */
{ 1.329834, -0.246888, -0.824571 }, /* lookat */ 

{ -2.410256, -2.019977, -9.836977 }, /* eye pos */
{ -1.703930, -1.318722, -7.006898 }, /* lookat */ 

{ -7.471500, -3.620900, -15.391470 }, /* eye pos */
{ -5.961942, -3.164474, -12.839430 }, /* lookat */ 

{ -4.160040, -2.025674, -5.488741 }, /* eye pos */
{ -2.842467, -1.541702, -2.837369 }, /* lookat */ 

{ -7.564082, -2.608361, -6.565232 }, /* eye pos */
{ -5.567526, -1.874560, -4.449740 }, /* lookat */ 

{ -11.353507, -3.168406, -4.889864 }, /* eye pos */
{ -8.884224, -2.330117, -3.406655 }, /* lookat */

{ -12.541506, -3.017866, -3.813373 }, /* eye pos */
{ -9.873917, -2.390616, -2.592502 }, /* lookat */

{ -13.830865, -3.017866, 0.589832 }, /* eye pos */
{ -11.107089, -2.247501, 1.583605 }, /* lookat */

{ -11.539225, -3.017866, -4.045604 }, /* eye pos */
{ -9.126908, -2.302515, -2.411896 }, /* lookat */

{ -6.775560, -2.961938, -6.462805 }, /* eye pos */
{ -5.019531, -2.512826, -4.072269 }, /* lookat */

{ -2.425160, -2.378819, -5.082704 }, /* eye pos */
{ -1.384145, -2.085273, -2.284470 }, /* lookat */

{ 0.194574, -2.378819, -5.891953 }, /* eye pos */
{ 0.033727, -2.101878, -2.909096 }, /* lookat */
};

camera_path wobble_tube_path = {
  .path = &c_path,
  .length = ARRAY_LENGTH (c_path),
  .step_time = 800
};

#define ROWS 48
#define SEGMENTS 32

static float rot1 = 0;
static float rot2 = 0;
static float rot3 = 0;
static float rot4 = 0;
static float rot5 = 0;
static float rot6 = 0;
static float rot7 = 0;

static object *tube;

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));
static object_orientation obj_orient = {
  .modelview = &mview,
  .normal_xform = &normxform,
  .dirty = 1
};

static pvr_ptr_t highlight;
static fakephong_info fphong;

static void
extra_wobbly_tube_fill (object *obj, int rows, int segments, float amp)
{
  int r, s;
  strip *strlist = obj->striplist;
  float points[ROWS + 1][SEGMENTS][3];
  float normals[ROWS + 1][SEGMENTS][3];
  
  for (r = 0; r < rows + 1; r++)
    {
      float ang = rot1 + 16 * (float) r / (float) rows;
      float mag = 1.0 + 0.35 * fsin (ang);

      for (s = 0; s < segments; s++)
        {
	  float sang = 2 * M_PI * (float) s / (float) segments;
	  float cosang = fcos (sang);
	  float sinang = fsin (sang);
	  
	  points[r][s][0] = mag * cosang + amp;
	  points[r][s][1] = (16.0 * (float) r / (float) rows) - 8.0;
	  points[r][s][2] = mag * sinang;
	  
	  points[r][s][0] += fsin (points[r][s][1] + rot2)
			     + fsin (0.7 * points[r][s][1] + rot4);
			     + fsin (0.6 * points[r][s][1] + rot6);
	  points[r][s][2] += fsin (points[r][s][1] + rot3)
			     + fsin (0.7 * points[r][s][1] + rot5);
			     + fsin (0.6 * points[r][s][1] + rot7);
	}
    }
  
  for (r = 0; r < rows; r++)
    {
      for (s = 0; s < segments; s++)
        {
	  int next_s = (s == segments - 1) ? 0 : s + 1;
	  float rvec[3], svec[3];
	  
	  vec_sub (&rvec[0], &points[r + 1][s][0], &points[r][s][0]);
	  vec_sub (&svec[0], &points[r][next_s][0], &points[r][s][0]);
	  vec_cross (&normals[r][s][0], &rvec[0], &svec[0]);
	  vec_normalize (&normals[r][s][0], &normals[r][s][0]);
	}
    }
  
  for (r = 0; r < rows; r++)
    {
      float (*str)[][3];
      float (*norm)[][3];

      assert (strlist);
      
      str = strlist->start;
      norm = strlist->normals;
      strlist->inverse = 0;
      
      for (s = 0; s <= segments; s++)
        {
	  int use_s = (s == segments) ? 0 : s;

	  (*str)[s * 2 + 1][0] = points[r][use_s][0];
	  (*str)[s * 2 + 1][1] = points[r][use_s][1];
	  (*str)[s * 2 + 1][2] = points[r][use_s][2];

	  (*norm)[s * 2 + 1][0] = normals[r][use_s][0];
	  (*norm)[s * 2 + 1][1] = normals[r][use_s][1];
	  (*norm)[s * 2 + 1][2] = normals[r][use_s][2];

	  (*str)[s * 2][0] = points[r + 1][use_s][0];
	  (*str)[s * 2][1] = points[r + 1][use_s][1];
	  (*str)[s * 2][2] = points[r + 1][use_s][2];

	  (*norm)[s * 2][0] = normals[r + 1][use_s][0];
	  (*norm)[s * 2][1] = normals[r + 1][use_s][1];
	  (*norm)[s * 2][2] = normals[r + 1][use_s][2];
	}
      
      strlist = strlist->next;
    }
}

static void
preinit_tube (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;
  
  tube = allocate_tube (ROWS, SEGMENTS);

  object_set_ambient (tube, 64, 0, 0);
  object_set_pigment (tube, 255, 0, 0);
  object_set_clipping (tube, 1);

  highlight = pvr_mem_malloc (128 * 128);
  fakephong_highlight_texture (highlight, 128, 128, 30.0);
  
  fphong.highlight = highlight;
  fphong.xsize = 128;
  fphong.ysize = 128;
  fphong.intensity = 255;
  
  tube->fake_phong = &fphong;
  
  initialised = 1;
}

static void
finalize_tube (void *params)
{
  pvr_mem_free (highlight);
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam,
	       viewpoint *view, lighting *lights)
{
  extra_wobbly_tube_fill (tube, ROWS, SEGMENTS, 0.0);

  light_set_pos (lights, 0, 0, 0, -15);

  rot1 += 0.05 + audio_amplitude (33) * 0.05;
  rot2 += 0.04;
  rot3 += 0.03;
  rot4 += 0.045;
  rot5 += 0.035;
  rot6 += 0.06;
  rot7 += 0.055;
}

static void
render_wobble_tube (uint32_t time_offset, void *params, int iparam,
		    viewpoint *view, lighting *lights, int pass)
{
#ifndef USE_DMA
  if (pass != PVR_LIST_OP_POLY)
    return;
#endif

  glKosMatrixDirty ();
  glMatrixMode (GL_MODELVIEW);

  glPushMatrix ();
  glTranslatef (0, 0, 4);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  glPopMatrix ();

  lights->active = 1;
#ifdef USE_DMA
  object_render_deferred (view, tube, &obj_orient, lights);
#else
  object_render_untextured_phong (view, tube, &obj_orient, lights, pass);
#endif
}

effect_methods wobble_tube_methods = {
  .preinit_assets = &preinit_tube,
  .init_effect = NULL,
  .prepare_frame = &prepare_frame,
  .display_effect = &render_wobble_tube,
  .uninit_effect = NULL,
  .finalize = &finalize_tube
};
