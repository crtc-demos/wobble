/* Some fire.  */

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>

#include "perlin.h"
#include "object.h"
#include "timing.h"

static float rot1 = 0.0;
static float perlin_phase = 0.0;
static float cooling_offset = 0.0;
static pvr_ptr_t warp_texture[2];
static pvr_ptr_t cooling_texture;
static int warp_active = 0;

static void
render_cooling_texture (void)
{
#define NOISEFN(X,Y) perlin_noise_2D ((float) (X) / 10, (float) (Y) / 10, 2)
  int x, y;
  float min_intens = 0.0, max_intens = 0.0;
  uint16 *cooltmp;
  
  dbgio_printf ("Making cooling texture... ");
  
  cooltmp = malloc (1024 * 512 * 2);
  
  for (y = 0; y < 512; y++)
    for (x = 0; x < 1024; x++)
      {
        float intensity = NOISEFN (x, y);
	
	if (intensity < min_intens)
	  min_intens = intensity;
	
	if (intensity > max_intens)
	  max_intens = intensity;
      }
  
  for (y = 0; y < 512; y++)
    for (x = 0; x < 1024; x++)
      {
        float intensity = NOISEFN (x, y);
	int alpha;
	
	/* Re-scale to 0...1.  */
	intensity = (intensity - min_intens) / (max_intens - min_intens);
	
	alpha = (int) (intensity * 15 + drand48 ()) - 8;

	if (alpha < 0)
	  alpha = 0;
	if (alpha > 15)
	  alpha = 15;
	
	cooltmp[y * 1024 + x] = alpha << 12;
      }

  pvr_txr_load_ex (cooltmp, cooling_texture, 1024, 512, PVR_TXRLOAD_16BPP);

  free (cooltmp);
  
  dbgio_printf ("done.\n");
#undef NOISEFN
}

#define DISTORT_DEPTH 0.12f
#define SEED_DEPTH 0.125f
#define COOLING_DEPTH DISTORT_DEPTH

static void
draw_box (void)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t poly;
  pvr_vertex_t vert;
  int x, y;
  int ox, oy;
  float offset[21][16][2];

  pvr_poly_cxt_txr (&cxt, PVR_LIST_OP_POLY,
		    PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, 1024, 512,
		    warp_texture[1 - warp_active], PVR_FILTER_BILINEAR);
  pvr_poly_compile (&poly, &cxt);
  
  for (oy = 0; oy <= 15; oy++)
    for (ox = 0; ox <= 20; ox++)
      {
        offset[ox][oy][0] = 2.0 * perlin_noise_2D (ox, oy + perlin_phase, 2);
	offset[ox][oy][1]
	  = 4.0 + 2.0 * perlin_noise_2D (ox + 40, oy + perlin_phase, 2);
      }
  
  for (oy = 0, y = 0; y < 480; oy++, y += 32)
    {
      pvr_prim (&poly, sizeof (poly));

      vert.flags = PVR_CMD_VERTEX;
      vert.x = offset[0][oy + 1][0];
      vert.y = y + 32 + offset[0][oy + 1][1];
      vert.z = DISTORT_DEPTH;
      vert.u = 0.0f;
      vert.v = (float) (y + 32) / 512.0f;
      vert.argb = 0xffffffff;
      vert.oargb = 0;
      pvr_prim (&vert, sizeof (vert));

      vert.x = offset[0][oy][0];
      vert.y = y + offset[0][oy][1];
      vert.v = (float) y / 512.0f;
      pvr_prim (&vert, sizeof (vert));

      for (ox = 1, x = 32; x < 640; ox++, x += 32)
        {
	  int is_last = (x == 640 - 32);

	  vert.x = x + offset[ox][oy + 1][0];
	  vert.y = y + 32 + offset[ox][oy + 1][1];
	  vert.u = (float) x / 1024.0f;
	  vert.v = (float) (y + 32) / 512.0f;
	  pvr_prim (&vert, sizeof (vert));

	  if (is_last)
	    vert.flags = PVR_CMD_VERTEX_EOL;
	    
	  vert.x = x + offset[ox][oy][0];
	  vert.y = y + offset[ox][oy][1];
	  vert.v = (float) y / 512.0f;
	  pvr_prim (&vert, sizeof (vert));
        }
    }

  pvr_poly_cxt_col (&cxt, PVR_LIST_OP_POLY);
  pvr_poly_compile (&poly, &cxt);
  
  pvr_prim (&poly, sizeof (poly));
  
  /* "Seed" square.  */
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 320 + fcos (rot1) * 50;
  vert.y = 240 + fsin (rot1) * 50;
  vert.z = SEED_DEPTH;
  vert.u = vert.v = 0.0f;
  vert.argb = 0xffff9944;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 320 + fcos (rot1 + M_PI / 2.0) * 50;
  vert.y = 240 + fsin (rot1 + M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 320 + fcos (rot1 + 3.0 * M_PI / 2.0) * 50;
  vert.y = 240 + fsin (rot1 + 3.0 * M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 320 + fcos (rot1 + M_PI) * 50;
  vert.y = 240 + fsin (rot1 + M_PI) * 50;
  pvr_prim (&vert, sizeof (vert));

#if 0
  /* And another...  */
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 480 + fcos (-rot1) * 50;
  vert.y = 120 + fsin (-rot1) * 50;
  vert.z = 6.0f;
  vert.u = vert.v = 0.0f;
  vert.argb = 0xff4499ff;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 480 + fcos (-rot1 + M_PI / 2.0) * 50;
  vert.y = 120 + fsin (-rot1 + M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 480 + fcos (-rot1 + 3.0 * M_PI / 2.0) * 50;
  vert.y = 120 + fsin (-rot1 + 3.0 * M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 480 + fcos (-rot1 + M_PI) * 50;
  vert.y = 120 + fsin (-rot1 + M_PI) * 50;
  pvr_prim (&vert, sizeof (vert));

  /* And another...  */
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 160 + fcos (-rot1) * 50;
  vert.y = 120 + fsin (-rot1) * 50;
  vert.z = 6.0f;
  vert.u = vert.v = 0.0f;
  vert.argb = 0xff99ff44;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 160 + fcos (-rot1 + M_PI / 2.0) * 50;
  vert.y = 120 + fsin (-rot1 + M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 160 + fcos (-rot1 + 3.0 * M_PI / 2.0) * 50;
  vert.y = 120 + fsin (-rot1 + 3.0 * M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 160 + fcos (-rot1 + M_PI) * 50;
  vert.y = 120 + fsin (-rot1 + M_PI) * 50;
  pvr_prim (&vert, sizeof (vert));
#endif

  rot1 += 0.05;
  if (rot1 > 2 * M_PI)
    rot1 -= 2 * M_PI;
  
  perlin_phase += 0.03;
}

static void
draw_texture (void)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t poly;
  pvr_vertex_t vert;
  
  pvr_poly_cxt_txr (&cxt, PVR_LIST_TR_POLY,
		    PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, 1024, 512,
		    warp_texture[1 - warp_active], PVR_FILTER_NONE);
  cxt.blend.src = PVR_BLEND_ONE;
  cxt.blend.dst = PVR_BLEND_ONE;
  pvr_poly_compile (&poly, &cxt);
  
  pvr_prim (&poly, sizeof (poly));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 0.0f;
  vert.y = 480.0f;
  vert.z = COOLING_DEPTH;
  vert.u = 0;
  vert.v = 0;
  vert.argb = 0xffffffff;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 0.0f;
  vert.y = 0.0f;
  vert.u = 0.0f;
  vert.v = 480.0f / 512.0f;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 640.0f;
  vert.y = 480.0f;
  vert.u = 640.0f / 1024.0f;
  vert.v = 0.0f;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 640.0f;
  vert.y = 0.0f;
  vert.u = 640.0f / 1024.0f;
  vert.v = 480.0f / 512.0f;
  pvr_prim (&vert, sizeof (vert));
  
  rot1 += 0.05;
  if (rot1 > 2 * M_PI)
    rot1 -= 2 * M_PI;
}

static void
draw_cooler (void)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t poly;
  pvr_vertex_t vert;
  
  pvr_poly_cxt_txr (&cxt, PVR_LIST_TR_POLY,
		    PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_TWIDDLED, 1024, 512,
		    cooling_texture, PVR_FILTER_NONE);
  pvr_poly_compile (&poly, &cxt);
  
  pvr_prim (&poly, sizeof (poly));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 0.0f;
  vert.y = 480.0f;
  vert.z = 7.0f;
  vert.u = 0.0f;
  vert.v = cooling_offset / 512.0f;
  vert.argb = 0xffffffff;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 0.0f;
  vert.y = 0.0f;
  vert.v = (cooling_offset + 480.0f) / 512.0f;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 640.0f;
  vert.y = 480.0f;
  vert.u = 640.0f / 1024.0f;
  vert.v = cooling_offset / 512.0f;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 640.0f;
  vert.y = 0.0f;
  vert.u = 640.0f / 1024.0f;
  vert.v = (cooling_offset + 480.0f) / 512.0f;
  pvr_prim (&vert, sizeof (vert));
  
  cooling_offset += 3;
  if (cooling_offset > 512)
    cooling_offset -= 512;
}

static void
preinit_fire (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;

  cooling_texture = pvr_mem_malloc (1024 * 512 * 2);

  render_cooling_texture ();
  
  initialised = 1;
}

static void
init_effect (void *params)
{
  warp_texture[0] = pvr_mem_malloc (1024 * 512 * 2);
  warp_texture[1] = pvr_mem_malloc (1024 * 512 * 2);
  memset (warp_texture[0], 0, 1024 * 512 * 2);
  memset (warp_texture[1], 0, 1024 * 512 * 2);
}

static void
uninit_effect (void *params)
{
  pvr_mem_free (warp_texture[0]);
  pvr_mem_free (warp_texture[1]);
  pvr_mem_free (cooling_texture);
}

static void
do_feedback (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	     lighting *lights)
{
  uint32 tx_x = 1024, tx_y = 512;

  /* Render to texture.  */
  pvr_wait_ready ();
  pvr_scene_begin_txr (warp_texture[warp_active], &tx_x, &tx_y);

  pvr_list_begin (PVR_LIST_OP_POLY);
  draw_box ();
  pvr_list_finish ();

  pvr_list_begin (PVR_LIST_TR_POLY);
  draw_cooler ();
  pvr_list_finish ();

  pvr_scene_finish ();
}


static void
render_fire (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	     lighting *lights, int pass)
{
  if (pass != 1)
    return;

  draw_texture ();

  warp_active = 1 - warp_active;
}

effect_methods fire_methods = {
  .preinit_assets = &preinit_fire,
  .init_effect = &init_effect,
  .prepare_frame = &do_feedback,
  .display_effect = &render_fire,
  .uninit_effect = &uninit_effect
};
