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
{ -8.862904, -14.704176, 18.480724 }, /* eye pos */       
{ -9.465591, -11.988854, 17.356537 }, /* lookat */        

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -10.289760, -12.271708, 17.457407 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -10.700147, -12.406928, 17.891497 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -10.903364, -12.507903, 18.367111 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -11.078581, -12.749616, 19.000788 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -11.033916, -12.952872, 19.585098 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -10.850843, -13.138194, 20.091871 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -10.442310, -13.372149, 20.655851 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -9.938469, -13.542644, 21.029058 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -9.304542, -13.735497, 21.285473 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -8.681345, -13.915401, 21.369473 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -8.123095, -14.075609, 21.319313 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -7.669184, -14.213355, 21.188885 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -7.265119, -14.344913, 20.994289 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -6.575682, -14.584867, 20.418348 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -6.184138, -14.728109, 19.831148 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -5.886209, -14.614935, 18.843117 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -5.898810, -14.723113, 18.018351 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -6.038647, -14.821057, 17.475780 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -6.980797, -14.812682, 16.147079 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -8.513677, -14.926075, 15.509394 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -10.016010, -14.756454, 15.711679 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -11.412876, -14.251174, 16.966648 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -11.834276, -14.305369, 18.371634 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -11.438311, -14.408417, 19.990625 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -10.366729, -14.482884, 21.067139 }, /* lookat */

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -9.134312, -14.563118, 21.465090 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -7.849298, -14.641856, 21.303616 }, /* lookat */ 

{ -8.862904, -14.704176, 18.480724 }, /* eye pos */
{ -6.699319, -14.704717, 20.558922 }, /* lookat */ 

{ -9.670945, -14.589871, 18.088861 }, /* eye pos */
{ -6.881538, -15.118945, 19.058031 }, /* lookat */ 

{ -10.492789, -14.468087, 17.920540 }, /* eye pos */
{ -7.520895, -14.763544, 18.204351 }, /* lookat */  

{ -12.611785, -14.642510, 18.351765 }, /* eye pos */
{ -10.029189, -13.906885, 17.014210 }, /* lookat */ 

{ -13.893704, -15.352118, 19.579332 }, /* eye pos */
{ -12.139647, -13.695449, 17.796432 }, /* lookat */ 

{ -13.452806, -14.935279, 19.133959 }, /* eye pos */
{ -11.695450, -13.273824, 17.358778 }, /* lookat */ 

{ -12.231136, -13.720240, 17.890486 }, /* eye pos */
{ -10.517302, -11.987167, 16.141418 }, /* lookat */ 

{ -10.677343, -12.148801, 16.304720 }, /* eye pos */
{ -8.966016, -10.411506, 14.557389 }, /* lookat */  

{ -9.300518, -10.674167, 14.888439 }, /* eye pos */
{ -7.628050, -8.874382, 13.166921 }, /* lookat */  

{ -8.163250, -9.450313, 13.717800 }, /* eye pos */
{ -6.490782, -7.650528, 11.996282 }, /* lookat */ 

{ -7.168132, -8.379440, 12.693491 }, /* eye pos */
{ -5.495664, -6.579656, 10.971972 }, /* lookat */ 

{ -6.125815, -7.257346, 11.620461 }, /* eye pos */
{ -4.460042, -5.449519, 9.900880 }, /* lookat */  

{ -5.086833, -6.132124, 10.547497 }, /* eye pos */
{ -3.407000, -4.358681, 8.805929 }, /* lookat */  

{ -4.602901, -5.077998, 10.044827 }, /* eye pos */
{ -2.912066, -3.329704, 8.288539 }, /* lookat */  

{ -6.101447, -5.273854, 11.601395 }, /* eye pos */
{ -4.410612, -3.525560, 9.845107 }, /* lookat */  

{ -7.251182, -5.314642, 12.002625 }, /* eye pos */
{ -5.560347, -3.566348, 10.246337 }, /* lookat */ 

{ -8.555917, -5.379678, 12.420671 }, /* eye pos */
{ -6.865082, -3.631384, 10.664383 }, /* lookat */ 

{ -10.324861, -5.528948, 13.034479 }, /* eye pos */
{ -8.641062, -3.798486, 11.253940 }, /* lookat */  

{ -11.753628, -5.490687, 13.618803 }, /* eye pos */
{ -10.013754, -4.381056, 11.441292 }, /* lookat */ 

{ -13.013562, -5.117135, 14.192807 }, /* eye pos */
{ -11.228708, -4.783123, 11.804768 }, /* lookat */ 

{ -14.093589, -4.678571, 14.532746 }, /* eye pos */
{ -11.840737, -4.317858, 12.584786 }, /* lookat */ 

{ -16.267284, -3.882015, 14.618679 }, /* eye pos */
{ -13.616158, -3.382872, 13.306285 }, /* lookat */ 

{ -17.833466, -3.298758, 14.501327 }, /* eye pos */
{ -15.182338, -2.799614, 13.188932 }, /* lookat */ 

{ -19.119308, -2.700572, 14.368325 }, /* eye pos */
{ -16.457806, -2.632263, 12.985671 }, /* lookat */ 

{ -20.062366, -2.168125, 14.238452 }, /* eye pos */
{ -17.400862, -2.099816, 12.855798 }, /* lookat */ 

{ -22.710613, -0.646064, 13.878454 }, /* eye pos */
{ -20.093527, -0.801446, 12.420129 }, /* lookat */ 

{ -24.641085, 0.825243, 13.537107 }, /* eye pos */
{ -22.025991, 0.350286, 12.145809 }, /* lookat */ 

{ -25.904144, 1.886686, 13.119269 }, /* eye pos */
{ -23.134520, 1.453419, 12.050880 }, /* lookat */ 

{ -26.755257, 2.637757, 12.719117 }, /* eye pos */
{ -23.907207, 2.235918, 11.866391 }, /* lookat */ 

{ -27.348612, 3.207118, 12.376300 }, /* eye pos */
{ -24.508802, 2.785907, 11.505634 }, /* lookat */ 

{ -28.871798, 4.828967, 11.404542 }, /* eye pos */
{ -26.105045, 4.183100, 10.441244 }, /* lookat */ 

{ -29.914070, 6.275211, 10.490377 }, /* eye pos */
{ -27.217953, 5.281563, 9.628048 }, /* lookat */  

{ -30.519674, 7.389709, 9.722694 }, /* eye pos */
{ -27.809437, 6.252197, 9.122129 }, /* lookat */ 

{ -30.909155, 8.246111, 9.092026 }, /* eye pos */
{ -28.198917, 7.108599, 8.491461 }, /* lookat */ 

{ -31.336840, 9.316525, 8.264963 }, /* eye pos */
{ -28.626602, 8.179012, 7.664398 }, /* lookat */ 

{ -31.868137, 10.687615, 7.196084 }, /* eye pos */
{ -29.103512, 9.575455, 6.849748 }, /* lookat */  

{ -32.264389, 11.714883, 6.235676 }, /* eye pos */
{ -29.496452, 10.604355, 5.911244 }, /* lookat */ 

{ -32.965649, 11.780514, 5.102899 }, /* eye pos */
{ -30.156347, 10.737124, 4.964449 }, /* lookat */ 

{ -33.618160, 12.003254, 3.926603 }, /* eye pos */
{ -30.803339, 10.970990, 3.820710 }, /* lookat */ 

{ -34.040066, 12.157296, 2.864019 }, /* eye pos */
{ -31.189451, 11.258794, 3.122263 }, /* lookat */ 

{ -34.237629, 12.238626, 2.193519 }, /* eye pos */
{ -31.393177, 11.319840, 2.448318 }, /* lookat */ 

{ -33.897419, 12.134497, 2.023194 }, /* eye pos */
{ -31.052967, 11.215711, 2.277993 }, /* lookat */ 

{ -31.346613, 11.350566, 0.746028 }, /* eye pos */
{ -28.509033, 10.411542, 1.003674 }, /* lookat */ 

{ -29.898739, 10.881737, 0.043081 }, /* eye pos */
{ -27.089659, 9.886114, 0.386307 }, /* lookat */  

{ -28.374172, 10.424043, -0.472750 }, /* eye pos */
{ -25.667864, 9.518453, 0.452343 }, /* lookat */   

{ -26.903238, 9.984682, -0.835694 }, /* eye pos */
{ -24.215902, 8.921856, -0.030322 }, /* lookat */ 

{ -24.556242, 9.191845, -1.496724 }, /* eye pos */
{ -21.871105, 8.115588, -0.701921 }, /* lookat */ 

{ -22.479923, 8.490520, -2.081071 }, /* eye pos */
{ -19.797016, 7.415339, -1.277328 }, /* lookat */ 

{ -20.748089, 7.916524, -2.505927 }, /* eye pos */
{ -18.117264, 6.861899, -1.522806 }, /* lookat */ 

{ -18.999310, 7.348785, -2.866574 }, /* eye pos */
{ -16.368484, 6.294160, -1.883454 }, /* lookat */ 

{ -17.794033, 6.966677, -3.075454 }, /* eye pos */
{ -15.231775, 5.958516, -1.884470 }, /* lookat */ 

{ -16.559305, 6.637434, -3.175858 }, /* eye pos */
{ -14.010809, 5.766790, -1.854062 }, /* lookat */ 

{ -15.323519, 6.309583, -3.269882 }, /* eye pos */
{ -12.779123, 5.404872, -1.963151 }, /* lookat */ 

{ -15.254992, 6.309583, -3.403313 }, /* eye pos */
{ -12.710596, 5.404872, -2.096582 }, /* lookat */ 

{ -14.683931, 6.309583, -4.515247 }, /* eye pos */
{ -12.139535, 5.404872, -3.208516 }, /* lookat */ 

{ -14.227083, 6.309583, -5.404796 }, /* eye pos */
{ -11.682687, 5.404872, -4.098065 }, /* lookat */ 

{ -13.849624, 6.309583, -6.110083 }, /* eye pos */
{ -11.353796, 5.416989, -4.705049 }, /* lookat */ 

{ -13.412558, 6.309583, -6.838991 }, /* eye pos */
{ -10.987918, 5.437395, -5.302628 }, /* lookat */ 

{ -12.879503, 6.309583, -7.625178 }, /* eye pos */
{ -10.543799, 5.461163, -5.944503 }, /* lookat */ 

{ -12.268954, 6.309583, -8.416948 }, /* eye pos */
{ -10.037011, 5.489970, -6.587559 }, /* lookat */ 

{ -11.678622, 6.309583, -9.096148 }, /* eye pos */
{ -9.545628, 5.509972, -7.143981 }, /* lookat */  

{ -11.017976, 6.309583, -9.778675 }, /* eye pos */
{ -8.986796, 5.527225, -7.714159 }, /* lookat */  

{ -9.484211, 6.125901, -10.215922 }, /* eye pos */
{ -7.616967, 5.375319, -7.991056 }, /* lookat */  

{ -7.768698, 5.727365, -9.577213 }, /* eye pos */
{ -6.032433, 4.991335, -7.244051 }, /* lookat */

{ -6.364660, 5.387285, -8.943508 }, /* eye pos */
{ -4.808033, 4.624640, -6.494981 }, /* lookat */

{ -5.013021, 5.035157, -8.210297 }, /* eye pos */
{ -3.646933, 4.245966, -5.658634 }, /* lookat */

{ -3.879572, 4.717155, -7.489441 }, /* eye pos */
{ -2.686840, 3.904243, -4.859505 }, /* lookat */

{ -2.349279, 4.245712, -6.339084 }, /* eye pos */
{ -1.410824, 3.395386, -3.619479 }, /* lookat */

{ -1.071766, 3.803346, -5.189555 }, /* eye pos */
{ -0.371085, 2.921789, -2.408925 }, /* lookat */

{ 0.710192, 3.087479, -3.207710 }, /* eye pos */
{ 1.068960, 2.185819, -0.368996 }, /* lookat */

{ 2.055065, 3.087479, -3.320280 }, /* eye pos */
{ 2.165923, 2.297960, -0.428157 }, /* lookat */

{ 3.403848, 3.087479, -3.287866 }, /* eye pos */
{ 3.162086, 2.431002, -0.370574 }, /* lookat */

{ 3.503447, 3.087479, -3.278917 }, /* eye pos */
{ 3.234560, 2.441122, -0.361740 }, /* lookat */

{ 3.503447, 3.087479, -3.278917 }, /* eye pos */
{ 2.823483, 2.478730, -0.421108 }, /* lookat */

{ 3.503447, 3.087479, -3.278917 }, /* eye pos */
{ 2.790401, 2.481022, -0.428693 }, /* lookat */

{ 3.503447, 3.087479, -3.278917 }, /* eye pos */
{ 2.790401, 2.481022, -0.428693 }, /* lookat */

{ 3.503447, 3.087479, -3.278917 }, /* eye pos */
{ 2.790401, 2.481022, -0.428693 }, /* lookat */
};

camera_path wobble_tube_path = {
  .path = &c_path,
  .length = ARRAY_LENGTH (c_path),
  .step_time = 500
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

  rot1 += 0.05 + audio_amplitude () * 0.05;
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
