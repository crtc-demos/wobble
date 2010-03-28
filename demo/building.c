#include <math.h>
#include <stdint.h>

#include <kos.h>
#include <kmg/kmg.h>
#include <png/png.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "vector.h"
#include "object.h"
#include "palette.h"
#include "timing.h"
#include "cam_path.h"
#include "loader.h"

static float building_path[][3] = {
{ 11.622311, -0.154918, 16.760166 }, /* eye pos */        
{ 10.282506, 1.972999, 15.124044 }, /* lookat */          

{ 11.622311, -0.154918, 16.760166 }, /* eye pos */
{ 10.190777, 1.820122, 15.013762 }, /* lookat */  

{ 11.622311, -0.154918, 16.760166 }, /* eye pos */
{ 10.010843, 1.448245, 14.802350 }, /* lookat */  

{ 11.622311, -0.154918, 16.760166 }, /* eye pos */
{ 9.855821, 0.995113, 14.625466 }, /* lookat */   

{ 11.622311, -0.154918, 16.760166 }, /* eye pos */
{ 9.880569, 0.579575, 14.430605 }, /* lookat */   

{ 11.622311, -0.154918, 16.760166 }, /* eye pos */
{ 9.956837, 0.373706, 14.321569 }, /* lookat */   

{ 11.622311, -0.154918, 16.760166 }, /* eye pos */
{ 10.039526, 0.051559, 14.220057 }, /* lookat */  

{ 11.102591, -0.172939, 15.799891 }, /* eye pos */
{ 9.762524, -0.289838, 13.118370 }, /* lookat */  

{ 10.419754, -0.241559, 14.418774 }, /* eye pos */
{ 9.136323, -0.437724, 11.714273 }, /* lookat */  

{ 9.547028, -0.374951, 12.579708 }, /* eye pos */
{ 8.263597, -0.571115, 9.875208 }, /* lookat */  

{ 8.708559, -0.500540, 10.818117 }, /* eye pos */
{ 7.421379, -0.692686, 8.115110 }, /* lookat */  

{ 7.826853, -0.630630, 8.983149 }, /* eye pos */
{ 6.500504, -0.819324, 6.298900 }, /* lookat */ 

{ 6.586699, -0.758069, 6.565814 }, /* eye pos */
{ 5.198465, -0.876098, 3.908960 }, /* lookat */ 

{ 5.667749, -0.814964, 4.841291 }, /* eye pos */
{ 4.231058, -0.846735, 2.207869 }, /* lookat */ 

{ 4.842231, -0.819293, 3.355289 }, /* eye pos */
{ 3.365753, -0.803319, 0.743823 }, /* lookat */ 

{ 3.480614, -0.745482, 1.240297 }, /* eye pos */
{ 1.576356, -0.509832, -1.065841 }, /* lookat */

{ 3.060051, -0.681210, 0.796191 }, /* eye pos */
{ 0.499537, -0.026126, -0.623186 }, /* lookat */

{ 1.937727, -0.371444, 0.233608 }, /* eye pos */
{ -0.803209, 0.554493, -0.560059 }, /* lookat */

{ 1.220653, -0.088619, 0.224741 }, /* eye pos */
{ -1.527936, 1.106217, 0.357505 }, /* lookat */ 

{ -0.479044, 0.719893, 0.674737 }, /* eye pos */
{ -3.098709, 1.980204, 1.415664 }, /* lookat */ 

{ -2.437379, 1.665081, 1.303094 }, /* eye pos */
{ -5.195619, 2.826427, 1.511394 }, /* lookat */ 

{ -3.420489, 1.895324, 1.149450 }, /* eye pos */
{ -6.285404, 2.435132, 0.441730 }, /* lookat */ 

{ -4.466298, 2.113610, 0.871580 }, /* eye pos */
{ -7.154404, 3.087214, -0.037365 }, /* lookat */

{ -5.648702, 2.596987, 0.288110 }, /* eye pos */
{ -8.019667, 3.691625, -1.188472 }, /* lookat */

{ -6.822127, 3.233502, -0.611253 }, /* eye pos */
{ -8.901325, 4.462986, -2.390384 }, /* lookat */ 

{ -7.809218, 3.708381, -1.607994 }, /* eye pos */
{ -9.752451, 4.520807, -3.744302 }, /* lookat */ 

{ -8.878523, 3.982603, -3.601011 }, /* eye pos */
{ -9.633163, 4.491310, -6.459636 }, /* lookat */ 

{ -8.912285, 4.123902, -4.344259 }, /* eye pos */
{ -7.668851, 4.811265, -6.986493 }, /* lookat */ 

{ -8.626867, 4.260630, -4.786771 }, /* eye pos */
{ -6.471476, 5.209655, -6.645167 }, /* lookat */ 

{ -7.232964, 4.915801, -5.709297 }, /* eye pos */
{ -4.820704, 6.151354, -6.995535 }, /* lookat */ 

{ -5.658949, 5.671931, -6.394722 }, /* eye pos */
{ -3.027818, 6.843501, -7.234112 }, /* lookat */ 

{ -4.564629, 6.169911, -6.727026 }, /* eye pos */
{ -1.926352, 7.432421, -7.394532 }, /* lookat */ 

{ -3.869668, 6.494229, -6.885360 }, /* eye pos */
{ -1.055469, 7.531990, -6.827617 }, /* lookat */ 

{ -2.788085, 6.884156, -6.708487 }, /* eye pos */
{ -0.082265, 7.975678, -6.010550 }, /* lookat */ 

{ -2.201170, 7.164522, -6.466241 }, /* eye pos */
{ 0.193048, 8.453133, -5.198485 }, /* lookat */  

{ -1.699813, 7.470908, -6.129537 }, /* eye pos */
{ 0.230529, 8.945530, -4.369062 }, /* lookat */  

{ -1.104217, 7.987572, -5.502694 }, /* eye pos */
{ 0.565815, 9.576628, -3.582825 }, /* lookat */  

{ -0.881098, 8.199088, -5.247329 }, /* eye pos */
{ 0.009346, 9.827566, -2.890389 }, /* lookat */  

{ -0.807615, 9.043639, -3.151335 }, /* eye pos */
{ -1.634938, 9.195555, -0.271672 }, /* lookat */ 

{ -0.947385, 9.489202, -2.458274 }, /* eye pos */
{ -2.785267, 9.142637, -0.112622 }, /* lookat */ 

{ -1.617229, 10.051909, -1.898757 }, /* eye pos */
{ -4.517345, 9.394347, -1.502601 }, /* lookat */  

{ -2.033900, 11.120321, -1.942802 }, /* eye pos */
{ -4.724476, 9.872513, -2.394217 }, /* lookat */  

{ -2.351329, 11.380846, -1.303976 }, /* eye pos */
{ -4.408092, 9.791484, -2.801858 }, /* lookat */  

{ -3.416104, 11.380846, -0.112947 }, /* eye pos */
{ -5.149317, 9.793515, -1.977446 }, /* lookat */

{ -5.148463, 11.380846, 1.236498 }, /* eye pos */
{ -6.454514, 9.788472, -0.944918 }, /* lookat */

{ -6.753829, 11.380846, 2.047224 }, /* eye pos */
{ -7.729073, 9.797569, -0.306950 }, /* lookat */

{ -8.461567, 10.708955, 2.857171 }, /* eye pos */
{ -8.992166, 9.519805, 0.154508 }, /* lookat */

{ -9.634207, 10.240492, 3.174791 }, /* eye pos */
{ -9.802971, 9.271480, 0.340618 }, /* lookat */

{ -10.589628, 10.571728, 3.108142 }, /* eye pos */
{ -10.564535, 9.645586, 0.254787 }, /* lookat */

{ -11.569889, 10.571728, 2.936224 }, /* eye pos */
{ -10.573036, 9.936908, 0.178818 }, /* lookat */

{ -12.236577, 10.571728, 2.601655 }, /* eye pos */
{ -10.216895, 10.250542, 0.406724 }, /* lookat */

{ -13.126151, 10.571728, 1.286188 }, /* eye pos */
{ -10.446533, 10.217287, -0.015358 }, /* lookat */

{ -13.484099, 10.571728, 0.146789 }, /* eye pos */
{ -10.526370, 10.399004, -0.324383 }, /* lookat */

{ -13.447280, 11.021399, -0.990837 }, /* eye pos */
{ -10.549940, 10.890852, -0.223778 }, /* lookat */

{ -13.040760, 11.021399, -1.841229 }, /* eye pos */
{ -10.800009, 10.533976, 0.093055 }, /* lookat */

{ -11.924918, 10.444321, -3.700075 }, /* eye pos */
{ -11.283944, 9.220448, -1.037127 }, /* lookat */

{ -10.287353, 9.737818, -3.510199 }, /* eye pos */
{ -12.457142, 9.566280, -1.445596 }, /* lookat */

{ -10.282816, 10.036253, -3.521478 }, /* eye pos */
{ -13.056873, 9.660779, -4.600191 }, /* lookat */

{ -10.282816, 10.036253, -3.521478 }, /* eye pos */
{ -12.267284, 9.806326, -5.759562 }, /* lookat */
};

camera_path waves_camera_path = {
  .path = &building_path,
  .length = ARRAY_LENGTH (building_path),
  .step_time = 1000
};

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));
static object_orientation obj_orient = {
  .modelview = &mview,
  .normal_xform = &normxform,
  .dirty = 1
};

static object *holder;
static pvr_ptr_t concrete_texture = 0;

static void
preinit_building (void)
{
  holder = load_object ("/rd/stair_things.strips");
  
  object_set_ambient (holder, 64, 64, 64);
  object_set_pigment (holder, 255, 255, 255);
  object_set_clipping (holder, 1);
}

static void
upload_texture (void *params)
{
  kos_img_t txr;

  kmg_to_img ("/rd/tex1.kmg", &txr);
  concrete_texture = pvr_mem_malloc (txr.byte_count);

  pvr_txr_load_kimg (&txr, concrete_texture, 0);
  object_set_all_textures (holder, concrete_texture, txr.w, txr.h,
			   PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED
			   | PVR_TXRFMT_RGB565);
  kos_img_free (&txr, 0);

  holder->textured = 1;
}

static void
free_texture (void *params)
{
  pvr_mem_free (concrete_texture);
}

static void
render_building (uint32_t time_offset, void *params, int iparam,
		 viewpoint *view, lighting *lights, int pass)
{
  if (pass != PVR_LIST_OP_POLY)
    return;

  glPushMatrix ();
  glTranslatef (0, -2, 0);
  glRotatef (30, 0, 1, 0);
  glScalef (1.7, 1, 1.7);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  object_render_immediate (view, holder, &obj_orient, lights, pass);
  glPopMatrix ();
}

effect_methods building_methods = {
  .preinit_assets = &preinit_building,
  .init_effect = &upload_texture,
  .prepare_frame = NULL,
  .display_effect = &render_building,
  .uninit_effect = &free_texture,
  .finalize = NULL
};
