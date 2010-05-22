#include <math.h>
#include <stdint.h>
#include <assert.h>

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
#include "cube.h"

static float building_path[][3] = {
{ 10.153307, -1.060856, 16.033731 }, /* eye pos */        
{ 8.364879, -0.716994, 13.649768 }, /* lookat */          

{ 8.286583, -0.751350, 12.665268 }, /* eye pos */
{ 7.373814, -0.613502, 9.810823 }, /* lookat */  

{ 6.832047, -0.544609, 7.803405 }, /* eye pos */
{ 5.579701, -0.538181, 5.077312 }, /* lookat */ 

{ 5.052123, -0.579928, 4.231414 }, /* eye pos */
{ 3.493209, -0.414534, 1.673596 }, /* lookat */ 

{ 3.190045, -0.391592, 1.271989 }, /* eye pos */
{ 0.895025, -0.229078, -0.653231 }, /* lookat */

{ 2.560667, 0.754160, -1.655730 }, /* eye pos */
{ 1.375182, 1.540244, 0.985614 }, /* lookat */  

{ 2.425012, 0.567629, -4.769232 }, /* eye pos */
{ 2.241262, 2.278472, -2.311743 }, /* lookat */ 

{ 2.425012, 0.567629, -4.769232 }, /* eye pos */
{ 0.951390, 1.580779, -2.360505 }, /* lookat */ 

{ 1.226162, 1.185328, -3.121680 }, /* eye pos */
{ -0.606147, 1.977971, -0.882403 }, /* lookat */

{ -0.287510, 1.722354, -1.529083 }, /* eye pos */
{ -2.447676, 2.377892, 0.446760 }, /* lookat */  

{ -2.094604, 2.462200, 0.355055 }, /* eye pos */
{ -4.816880, 2.981712, 1.503670 }, /* lookat */ 

{ -2.121047, 2.467468, 0.365732 }, /* eye pos */
{ -5.016251, 3.097082, 0.836238 }, /* lookat */ 

{ -3.376467, 3.080933, 0.449034 }, /* eye pos */
{ -6.291892, 3.750360, 0.220626 }, /* lookat */ 

{ -4.759473, 3.400595, 0.242176 }, /* eye pos */
{ -7.544120, 4.042346, -0.671005 }, /* lookat */

{ -6.109570, 3.742383, -0.483192 }, /* eye pos */
{ -8.358565, 4.404891, -2.354849 }, /* lookat */ 

{ -7.347709, 4.176395, -1.699830 }, /* eye pos */
{ -8.963017, 4.727718, -4.166978 }, /* lookat */ 

{ -7.110686, 4.749313, -3.919419 }, /* eye pos */
{ -8.080866, 5.120204, -6.733880 }, /* lookat */ 

{ -6.139090, 5.394325, -4.169337 }, /* eye pos */
{ -7.109270, 5.765216, -6.983798 }, /* lookat */ 

{ -5.598970, 5.890489, -4.290135 }, /* eye pos */
{ -6.284129, 6.194630, -7.194968 }, /* lookat */ 

{ -5.603279, 6.333598, -4.211797 }, /* eye pos */
{ -5.100922, 6.752722, -7.139590 }, /* lookat */ 

{ -4.975214, 6.936453, -5.582327 }, /* eye pos */
{ -3.491547, 7.261515, -8.169423 }, /* lookat */ 

{ -4.975214, 6.936453, -5.582327 }, /* eye pos */
{ -2.222075, 7.305825, -6.715371 }, /* lookat */ 

{ -3.254498, 7.198827, -6.010290 }, /* eye pos */
{ -0.311906, 7.748095, -5.811651 }, /* lookat */ 

{ -2.422385, 7.419579, -5.816380 }, /* eye pos */
{ 0.038076, 8.448475, -4.442513 }, /* lookat */  

{ -1.621994, 7.851140, -5.248400 }, /* eye pos */
{ 0.501046, 9.123671, -3.553299 }, /* lookat */  

{ -0.784093, 8.459841, -4.466217 }, /* eye pos */
{ 1.069349, 9.917217, -2.611276 }, /* lookat */  

{ -0.784093, 8.459841, -4.466217 }, /* eye pos */
{ 0.991697, 9.775129, -2.437280 }, /* lookat */  

{ 0.413826, 9.318983, -3.023576 }, /* eye pos */
{ 1.617075, 9.696249, -0.301469 }, /* lookat */ 

{ 0.608165, 9.276851, -1.804643 }, /* eye pos */
{ -0.356818, 9.617400, 1.015434 }, /* lookat */ 

{ 0.619234, 9.924797, -1.837737 }, /* eye pos */
{ -1.489693, 9.311012, 0.205715 }, /* lookat */ 

{ 0.137626, 10.535248, -1.727493 }, /* eye pos */
{ -2.585621, 9.440663, -1.106359 }, /* lookat */ 

{ -0.923755, 10.150916, -1.602869 }, /* eye pos */
{ -3.821603, 9.375845, -1.561121 }, /* lookat */  

{ -3.155974, 9.724260, -1.858183 }, /* eye pos */
{ -6.070302, 9.583740, -2.555999 }, /* lookat */ 

{ -4.373081, 9.936514, -1.271456 }, /* eye pos */
{ -6.714301, 9.568969, -3.110913 }, /* lookat */ 

{ -5.127797, 9.936514, -0.634595 }, /* eye pos */
{ -6.055627, 9.126059, -3.369974 }, /* lookat */ 

{ -6.200704, 9.936514, -0.434130 }, /* eye pos */
{ -5.780518, 8.565031, -3.068989 }, /* lookat */ 

{ -10.821490, 10.722201, 0.402787 }, /* eye pos */
{ -8.199242, 9.537020, -0.445243 }, /* lookat */  

{ -10.981377, 10.722201, -0.225979 }, /* eye pos */
{ -8.207243, 9.632264, -0.566882 }, /* lookat */   

{ -12.943316, 10.599814, -4.201686 }, /* eye pos */
{ -10.623933, 8.992655, -3.183109 }, /* lookat */  

{ -11.824703, 10.729013, -5.604877 }, /* eye pos */
{ -10.212366, 9.135132, -3.640211 }, /* lookat */  

{ -10.066896, 10.993552, -7.072172 }, /* eye pos */
{ -8.918713, 9.479084, -4.750952 }, /* lookat */   

{ -8.852410, 10.993552, -7.524443 }, /* eye pos */
{ -8.173752, 9.547797, -4.984912 }, /* lookat */  

{ -7.419919, 10.993552, -7.736230 }, /* eye pos */
{ -7.367713, 9.526503, -5.119927 }, /* lookat */  

{ -5.830966, 10.993552, -7.575129 }, /* eye pos */
{ -6.403255, 9.660131, -4.949396 }, /* lookat */  

{ -4.525384, 10.993552, -7.082180 }, /* eye pos */
{ -5.782557, 9.679507, -4.696220 }, /* lookat */  

{ -3.344684, 10.993552, -6.166963 }, /* eye pos */
{ -5.399589, 9.699050, -4.405817 }, /* lookat */  

{ -2.870418, 11.686907, -4.241396 }, /* eye pos */
{ -5.274427, 9.915358, -3.954421 }, /* lookat */  

{ -2.863882, 11.686907, -1.705238 }, /* eye pos */
{ -4.879134, 9.668566, -2.635325 }, /* lookat */

{ -3.496316, 12.646496, 1.132027 }, /* eye pos */
{ -5.094919, 10.590561, -0.357135 }, /* lookat */

{ -5.910053, 12.709944, 2.520076 }, /* eye pos */
{ -6.736940, 10.811818, 0.349045 }, /* lookat */

{ -8.470177, 13.490094, 3.981320 }, /* eye pos */
{ -7.769043, 11.650692, 1.717471 }, /* lookat */

{ -9.347013, 13.490094, 3.778558 }, /* eye pos */
{ -8.838603, 12.749101, 0.916313 }, /* lookat */

{ -10.318819, 13.490094, 3.554318 }, /* eye pos */
{ -9.265929, 14.020786, 0.795733 }, /* lookat */

{ -11.075551, 13.490094, 3.076683 }, /* eye pos */
{ -8.768363, 14.894316, 1.770910 }, /* lookat */
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
static pvr_ptr_t texture[4] = {0, 0, 0, 0};

static object *floor_box;

static void
preinit_building (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;

  holder = load_object ("/rd/stair_things.strips");
  
  object_set_ambient (holder, 64, 64, 64);
  object_set_pigment (holder, 255, 255, 255);
  object_set_clipping (holder, 1);

  floor_box = cube_create (100);
  object_set_ambient (floor_box, 32, 32, 32);
  object_set_pigment (floor_box, 96, 96, 96);
  object_set_clipping (floor_box, 1);
  
  initialised = 1;
}

static void
upload_texture (void *params)
{
  kos_img_t txr;
  strip *iter;

  kmg_to_img ("/rd/plain1.kmg", &txr);
  texture[0] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, texture[0], 0);
  kos_img_free (&txr, 0);

  kmg_to_img ("/rd/shouts1.kmg", &txr);
  texture[1] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, texture[1], 0);
  kos_img_free (&txr, 0);

  kmg_to_img ("/rd/shouts2.kmg", &txr);
  texture[2] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, texture[2], 0);
  kos_img_free (&txr, 0);

  kmg_to_img ("/rd/shouts3.kmg", &txr);
  texture[3] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, texture[3], 0);
  kos_img_free (&txr, 0);

  /* Resolve textures.  */
  for (iter = holder->striplist; iter; iter = iter->next)
    {
      assert (iter->s_attrs);
      iter->s_attrs->xsize = 1024;
      iter->s_attrs->ysize = 1024;
      iter->s_attrs->txr_fmt = PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED
			      | PVR_TXRFMT_RGB565;
      iter->s_attrs->texture = texture[iter->s_attrs->txr_idx];
    }

  holder->textured = 1;
}

static void
free_texture (void *params)
{
  int i;
  
  for (i = 0; i < 4; i++)
    pvr_mem_free (texture[i]);
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam,
	       viewpoint *view, lighting *lights)
{
  light_set_pos (lights, 0, 0, 15, 3);
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
  
  glPushMatrix ();
  glTranslatef (0, -102, 0);
  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  vec_normal_from_modelview (&normxform[0][0], &mview[0][0]);
  object_render_immediate (view, floor_box, &obj_orient, lights, pass);
  glPopMatrix ();
}

effect_methods building_methods = {
  .preinit_assets = &preinit_building,
  .init_effect = &upload_texture,
  .prepare_frame = &prepare_frame,
  .display_effect = &render_building,
  .uninit_effect = &free_texture,
  .finalize = NULL
};
