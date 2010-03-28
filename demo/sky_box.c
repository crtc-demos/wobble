/* A sky box.  */

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

static object *skybox;
static pvr_ptr_t skytex[6];

static matrix_t mview __attribute__((aligned(32)));
static matrix_t normxform __attribute__((aligned(32)));
static object_orientation obj_orient = {
  .modelview = &mview,
  .normal_xform = &normxform,
  .dirty = 1
};

static void
preinit_skybox (void)
{
  static int initialised = 0;
  kos_img_t txr;
  
  if (initialised)
    return;

  printf ("Loading sky box texture");
  fflush (stdout);
#if 0
  skytex[0] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky23.png", skytex[0], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[1] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky24.png", skytex[1], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[2] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky27.png", skytex[2], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[3] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky28.png", skytex[3], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[4] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky25.png", skytex[4], PNG_NO_ALPHA);
  putchar ('.');
  fflush (stdout);
  skytex[5] = pvr_mem_malloc (512 * 512 * 2);
  png_to_texture ("/rd/sky26.png", skytex[5], PNG_NO_ALPHA);
  putchar ('.');
#elif 0
  skytex[0] = pvr_mem_malloc (512 * 512 * 2);
  jpeg_to_texture ("/rd/sky23.jpg", skytex[0], 512, 1);
  putchar ('.');
  fflush (stdout);
  skytex[1] = pvr_mem_malloc (512 * 512 * 2);
  jpeg_to_texture ("/rd/sky24.jpg", skytex[1], 512, 1);
  putchar ('.');
  fflush (stdout);
  skytex[2] = pvr_mem_malloc (512 * 512 * 2);
  jpeg_to_texture ("/rd/sky27.jpg", skytex[2], 512, 1);
  putchar ('.');
  fflush (stdout);
  skytex[3] = pvr_mem_malloc (512 * 512 * 2);
  jpeg_to_texture ("/rd/sky28.jpg", skytex[3], 512, 1);
  putchar ('.');
  fflush (stdout);
  skytex[4] = pvr_mem_malloc (512 * 512 * 2);
  jpeg_to_texture ("/rd/sky25.jpg", skytex[4], 512, 1);
  putchar ('.');
  fflush (stdout);
  skytex[5] = pvr_mem_malloc (512 * 512 * 2);
  jpeg_to_texture ("/rd/sky26.jpg", skytex[5], 512, 1);
  putchar ('.');
#else
  kmg_to_img ("/rd/sky23.kmg", &txr);
  skytex[0] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, skytex[0], PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);
  
  kmg_to_img ("/rd/sky24.kmg", &txr);
  skytex[1] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, skytex[1], PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);
  
  kmg_to_img ("/rd/sky27.kmg", &txr);
  skytex[2] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, skytex[2], PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);
  
  kmg_to_img ("/rd/sky28.kmg", &txr);
  skytex[3] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, skytex[3], PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);
  
  kmg_to_img ("/rd/sky25.kmg", &txr);
  skytex[4] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, skytex[4], PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);
  
  kmg_to_img ("/rd/sky26.kmg", &txr);
  skytex[5] = pvr_mem_malloc (txr.byte_count);
  pvr_txr_load_kimg (&txr, skytex[5], PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);
#endif
  putchar ('\n');

  skybox = create_skybox (200, skytex, 512, 512, PVR_TXRFMT_RGB565
			  | PVR_TXRFMT_TWIDDLED | PVR_TXRFMT_VQ_ENABLE);

  initialised = 1;
}

static void
render_skybox (uint32_t time_offset, void *params, int iparam,
	       viewpoint *view, lighting *lights, int pass)
{
  if (pass != PVR_LIST_OP_POLY)
    return;

  glGetFloatv (GL_MODELVIEW_MATRIX, &mview[0][0]);
  object_render_immediate (view, skybox, &obj_orient, lights, pass);
}

static void
finalize_skybox (void *params)
{
  int i;
  
  for (i = 0; i < 6; i++)
    pvr_mem_free (skytex[i]);
}

effect_methods skybox_methods = {
  .preinit_assets = &preinit_skybox,
  .init_effect = NULL,
  .prepare_frame = NULL,
  .display_effect = &render_skybox,
  .uninit_effect = NULL,
  .finalize = &finalize_skybox
};
