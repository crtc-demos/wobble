/* Shiny thing.  */

#include <stdint.h>

#include <kos.h>
#include <png/png.h>

#include "timing.h"
#include "object.h"
#include "loader.h"
#include "cam_path.h"

static float c_path[][3] = {
{ -0.281392, -3.215581, -21.210672 }, /* eye pos */       
{ -0.148331, -2.796478, -18.243073 }, /* lookat */        

{ -7.501521, -2.320094, -19.476986 }, /* eye pos */
{ -6.351720, -2.104050, -16.714508 }, /* lookat */ 

{ -12.090624, -0.121004, -16.931856 }, /* eye pos */
{ -10.572203, -0.261160, -14.348302 }, /* lookat */ 

{ -17.275860, 5.368713, -14.976938 }, /* eye pos */
{ -14.989997, 4.634089, -13.178284 }, /* lookat */ 

{ -21.765503, 11.372513, -11.622918 }, /* eye pos */
{ -19.319651, 10.124004, -10.415013 }, /* lookat */ 

{ -25.435194, 11.394523, -8.217340 }, /* eye pos */
{ -22.773851, 10.421985, -7.231731 }, /* lookat */ 

{ -33.901661, 9.038757, -6.133427 }, /* eye pos */
{ -30.966167, 8.582466, -5.715478 }, /* lookat */ 

{ -32.516750, 1.445263, 1.392079 }, /* eye pos */
{ -29.557184, 0.972257, 1.260810 }, /* lookat */ 

{ -24.256359, -3.155162, 4.969872 }, /* eye pos */
{ -21.398029, -2.786910, 4.136601 }, /* lookat */ 

{ -17.107067, -5.786536, 6.254431 }, /* eye pos */
{ -14.404854, -5.235822, 5.073432 }, /* lookat */ 

{ -14.134155, -7.731292, 16.208126 }, /* eye pos */
{ -12.399985, -6.879233, 13.913208 }, /* lookat */ 

{ -10.255685, -5.744366, 24.271284 }, /* eye pos */
{ -9.188414, -5.080961, 21.547165 }, /* lookat */  

{ -4.338218, -3.302118, 22.819176 }, /* eye pos */
{ -3.739322, -2.855789, 19.913645 }, /* lookat */ 

{ -1.830944, -2.955100, 23.291550 }, /* eye pos */
{ -1.519035, -2.572062, 20.332497 }, /* lookat */ 

{ 0.999064, 0.130165, 23.590223 }, /* eye pos */
{ 1.269915, -0.266353, 20.628904 }, /* lookat */

{ 4.493917, 3.670780, 23.011496 }, /* eye pos */
{ 4.262879, 2.976810, 20.102024 }, /* lookat */ 

{ 8.133515, 7.484949, 20.992571 }, /* eye pos */
{ 7.270453, 6.430193, 18.320004 }, /* lookat */ 

{ 11.670265, 12.309011, 16.547033 }, /* eye pos */
{ 10.116167, 10.916963, 14.391348 }, /* lookat */ 

{ 14.240775, 13.100950, 13.547824 }, /* eye pos */
{ 12.616686, 11.178839, 11.914477 }, /* lookat */ 

{ 20.574415, 9.116070, 8.185827 }, /* eye pos */
{ 17.987419, 7.909214, 7.263354 }, /* lookat */ 

{ 21.012730, 7.809330, 0.560699 }, /* eye pos */
{ 18.192261, 6.800771, 0.394091 }, /* lookat */ 

{ 13.013100, 5.052905, -3.640843 }, /* eye pos */
{ 10.278235, 4.001081, -2.997272 }, /* lookat */ 

{ 18.498768, 8.539788, -12.866062 }, /* eye pos */
{ 16.371086, 7.336826, -11.126563 }, /* lookat */ 

{ 18.184933, 5.868601, -14.269665 }, /* eye pos */
{ 16.144487, 5.140882, -12.194330 }, /* lookat */ 

{ 16.189249, 3.443356, -13.507004 }, /* eye pos */
{ 14.370617, 2.374354, -11.373974 }, /* lookat */ 

{ 16.651773, 0.623150, -13.958517 }, /* eye pos */
{ 14.435566, 0.221703, -11.976785 }, /* lookat */ 

{ 16.682762, -2.670110, -13.982258 }, /* eye pos */
{ 14.575508, -2.532942, -11.851375 }, /* lookat */ 

{ 17.775972, -7.337569, -17.316444 }, /* eye pos */
{ 15.811340, -6.821754, -15.108696 }, /* lookat */ 

{ 18.608101, -9.326836, -19.589968 }, /* eye pos */
{ 16.800056, -8.382030, -17.390350 }, /* lookat */ 

{ 14.050355, -8.921682, -7.728260 }, /* eye pos */
{ 11.806828, -7.824285, -6.066241 }, /* lookat */ 

{ 12.176926, -9.754786, -6.249879 }, /* eye pos */
{ 10.189713, -8.139198, -4.687553 }, /* lookat */

{ 13.457921, -9.300105, -5.080069 }, /* eye pos */
{ 11.458085, -7.639764, -3.582096 }, /* lookat */

{ 15.615751, -8.831499, -2.487993 }, /* eye pos */
{ 13.122547, -7.447338, -1.556313 }, /* lookat */

{ 15.819280, -10.476872, 2.337926 }, /* eye pos */
{ 13.153524, -9.101272, 2.299560 }, /* lookat */

{ 16.270884, -7.185150, 5.523971 }, /* eye pos */
{ 13.349535, -6.819567, 4.947711 }, /* lookat */

{ 15.393548, -4.160126, 8.456467 }, /* eye pos */
{ 12.798918, -3.591218, 7.062107 }, /* lookat */

{ 14.343164, -1.579404, 10.839268 }, /* eye pos */
{ 11.949753, -1.744969, 9.038111 }, /* lookat */
};

camera_path shiny_thing_cam_path = {
  .path = &c_path,
  .length = ARRAY_LENGTH (c_path),
  .step_time = 300
};

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));
static object_orientation obj_orient = {
  .modelview = &mview,
  .normal_xform = &normxform,
  .dirty = 1
};

static object *thing;
static envmap_dual_para_info envmap;

void
preinit_effect (void)
{
  thing = load_object ("/rd/potato.strips");
  
  envmap.front_txr = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky21.png", envmap.front_txr, PNG_NO_ALPHA);
  envmap.front_txr_fmt = PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED;
  
  envmap.back_txr = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky22o.png", envmap.back_txr, PNG_MASK_ALPHA);
  envmap.back_txr_fmt = PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_TWIDDLED;
  
  envmap.xsize = 512;
  envmap.ysize = 512;
  
  thing->env_map = &envmap;
}

void
free_textures (void *params)
{
  pvr_mem_free (envmap.back_txr);
  pvr_mem_free (envmap.front_txr);
}

static void
display_shiny_thing (uint32_t time_offset, void *params, int iparam,
		     viewpoint *view, lighting *lights, int pass)
{
  float rot = (float) time_offset / 10.0;
  float amp = audio_amplitude (67) / 4.0;

  if (pass != PVR_LIST_OP_POLY && pass != PVR_LIST_PT_POLY)
    return;

  glPushMatrix ();
  glRotatef (rot, 0, 1, 0);
  glScalef (1.0 + amp, 1.0 + amp, 1.0 + amp);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  glPopMatrix ();

  object_render_immediate (view, thing, &obj_orient, lights, pass);
}

effect_methods shiny_thing_methods = {
  .preinit_assets = &preinit_effect,
  .init_effect = NULL,
  .prepare_frame = NULL,
  .display_effect = &display_shiny_thing,
  .uninit_effect = NULL,
  .finalize = &free_textures
};
