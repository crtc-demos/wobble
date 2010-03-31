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

static pvr_ptr_t flame_texture;
static pvr_ptr_t backdrop_texture;

static uint16 *cooltmp = 0;

#if 0
static pvr_poly_hdr_t user_clip = {
  PVR_CMD_USERCLIP, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};
#endif

float flame_pos[3];
static float flame_pos_xformed[4];

#define COOL_X 512
#define COOL_Y 256

static void
render_cooling_texture (void)
{
#define NOISEFN(X,Y) perlin_noise_2D ((float) (X) / 5, (float) (Y) / 5, 2)
  int x, y;
  float min_intens = 0.0, max_intens = 0.0;
  
  dbgio_printf ("Making cooling texture... ");
  
  cooltmp = malloc (COOL_X * COOL_Y * 2);
  
  for (y = 0; y < 256; y++)
    for (x = 0; x < 512; x++)
      {
        float intensity = NOISEFN (x, y);
	
	if (intensity < min_intens)
	  min_intens = intensity;
	
	if (intensity > max_intens)
	  max_intens = intensity;
      }
  
  for (y = 0; y < COOL_Y; y++)
    for (x = 0; x < COOL_X; x++)
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
	
	cooltmp[y * 512 + x] = alpha << 12;
      }

  
  dbgio_printf ("done.\n");
#undef NOISEFN
}

#define DISTORT_DEPTH 0.12f
#define SEED_DEPTH 0.125f
#define COOLING_DEPTH 0.12f

#define FLAME_DRAW_DEPTH 0.12f

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
	  = -4.0 - 2.0 * perlin_noise_2D (ox + 40, oy + perlin_phase, 2);
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

  rot1 += 0.005;
  if (rot1 > 2 * M_PI)
    rot1 -= 2 * M_PI;
  
  perlin_phase += 0.03;
}

static void
draw_backdrop (void)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t poly;
  pvr_vertex_t vert;
  
  pvr_poly_cxt_txr (&cxt, PVR_LIST_OP_POLY,
		    PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED, 512, 256,
		    backdrop_texture, PVR_FILTER_BILINEAR);
  pvr_poly_compile (&poly, &cxt);
  
  pvr_prim (&poly, sizeof (poly));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 0.0f;
  vert.y = 480.0f;
  vert.z = 0.001f;
  vert.u = 0.0f;
  vert.v = 1.0f;
  vert.argb = 0xff505050;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 0.0f;
  vert.y = 0.0f;
  vert.u = 0.0f;
  vert.v = 0.0f;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 640.0f;
  vert.y = 480.0f;
  vert.u = 1.0f;
  vert.v = 1.0f;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 640.0f;
  vert.y = 0.0f;
  vert.u = 1.0f;
  vert.v = 0.0f;
  pvr_prim (&vert, sizeof (vert));
}

static void
draw_texture (void)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t poly;
  pvr_vertex_t vert;
  float slice_width = 127.0;
  
  pvr_poly_cxt_txr (&cxt, PVR_LIST_TR_POLY,
		    PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, 1024, 512,
		    warp_texture[1 - warp_active], PVR_FILTER_NONE);
  cxt.blend.src = PVR_BLEND_ONE;
  cxt.blend.dst = PVR_BLEND_INVDESTCOLOR;
  cxt.txr.env = PVR_TXRENV_MODULATE;
  pvr_poly_compile (&poly, &cxt);
  
  pvr_prim (&poly, sizeof (poly));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 320.0f - slice_width;
  vert.y = 480.0f;
  vert.z = flame_pos_xformed[2];
  vert.u = (320.0f - slice_width) / 1024.0f;
  vert.v = 480.0f / 512.0f;
  vert.argb = 0xffffffff;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 320.0f - slice_width;
  vert.y = 0.0f;
  vert.u = (320.0f - slice_width) / 1024.0f;
  vert.v = 0.0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 320.0f + slice_width;
  vert.y = 480.0f;
  vert.u = (320.0f + slice_width) / 1024.0f;
  vert.v = 480.0f / 512.0f;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 320.0f + slice_width;
  vert.y = 0.0f;
  vert.u = (320.0f + slice_width) / 1024.0f;
  vert.v = 0.0f;
  pvr_prim (&vert, sizeof (vert));
}


static void
draw_cooler (void)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t poly;
  pvr_vertex_t vert;
  
  pvr_poly_cxt_txr (&cxt, PVR_LIST_TR_POLY,
		    PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_TWIDDLED, COOL_X, COOL_Y,
		    cooling_texture, PVR_FILTER_BILINEAR);
  pvr_poly_compile (&poly, &cxt);
  
  pvr_prim (&poly, sizeof (poly));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 0.0f;
  vert.y = 480.0f;
  vert.z = COOLING_DEPTH;
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

  /* Draw the flame bit too!  */
  {
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    int i;
    
    pvr_poly_cxt_txr (&cxt, PVR_LIST_TR_POLY,
		      PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
		      256, 256, flame_texture, PVR_FILTER_BILINEAR);
    cxt.blend.src = PVR_BLEND_ONE;
    cxt.blend.dst = PVR_BLEND_ONE;
    cxt.txr.env = PVR_TXRENV_MODULATE;
    pvr_poly_compile (&hdr, &cxt);
    
    pvr_prim (&hdr, sizeof (hdr));
    
    for (i = 0; i < 20; i++)
      {
        float ang = (float) i * 2 * M_PI / 20.0f;
	float ang1 = (float) (i + 1) * 2 * M_PI / 20.0f;
	float amp = audio_amplitude ();

        vert.flags = PVR_CMD_VERTEX;
	vert.x = flame_pos_xformed[0];
	vert.y = flame_pos_xformed[1];
	vert.z = SEED_DEPTH;
	vert.u = vert.v = 0.5;
	vert.argb = 0xffffffff;
	vert.oargb = 0x0;
	pvr_prim (&vert, sizeof (vert));
	
	vert.flags = PVR_CMD_VERTEX;
	vert.x = flame_pos_xformed[0] + fcos (ang) * (40 + amp * 10);
	vert.y = flame_pos_xformed[1] - 30 + fsin (ang) * 60;
	vert.u = 0.5 + fcos (rot1) * 0.5;
	vert.v = 0.5 + fsin (rot1) * 0.5;
	pvr_prim (&vert, sizeof (vert));
	
	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.x = flame_pos_xformed[0] + fcos (ang1) * (40 + amp * 10);
	vert.y = flame_pos_xformed[1] - 30 + fsin (ang1) * 60;
	pvr_prim (&vert, sizeof (vert));
      }
  }

  cooling_offset -= 3;
  if (cooling_offset < 0)
    cooling_offset += 512;
}

static void
preinit_fire (void)
{
  static int initialised = 0;
  
  if (initialised)
    return;

  render_cooling_texture ();
  
  initialised = 1;
}

static void
init_effect (void *params)
{
  pvr_mem_print_list ();
  // pvr_mem_reset ();

  warp_texture[0] = pvr_mem_malloc (1024 * 512 * 2);
  warp_texture[1] = pvr_mem_malloc (1024 * 512 * 2);
  memset (warp_texture[0], 0, 1024 * 512 * 2);
  memset (warp_texture[1], 0, 1024 * 512 * 2);

  if (cooltmp)
    {
      cooling_texture = pvr_mem_malloc (COOL_X * COOL_Y * 2);
      pvr_txr_load_ex (cooltmp, cooling_texture, COOL_X, COOL_Y,
		       PVR_TXRLOAD_16BPP);
      free (cooltmp);
      cooltmp = 0;
    }

  flame_texture = pvr_mem_malloc (256 * 256 * 2);
  png_to_texture ("/rd/flametex.png", flame_texture, PNG_NO_ALPHA);
  
  backdrop_texture = pvr_mem_malloc (512 * 256 * 2);
  png_to_texture ("/rd/backdrop.png", backdrop_texture, PNG_NO_ALPHA);
  //printf ("memory free: %d bytes\n", pvr_mem_available ());
}

static void
uninit_effect (void *params)
{
  pvr_mem_free (warp_texture[0]);
  pvr_mem_free (warp_texture[1]);
}

static void
finalize_effect (void *params)
{
  pvr_mem_free (cooling_texture);
  pvr_mem_free (flame_texture);
  pvr_mem_free (backdrop_texture);
}

static void
do_feedback (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	     lighting *lights)
{
  uint32 tx_x = 1024, tx_y = 512;
  float tmp[4], invw;
  
  flame_pos[0] = 0;
  flame_pos[1] = (float) (4500 - ((int) time_offset % 9000)) / 1100.0;
  flame_pos[2] = 4;

  view_set_eye_pos (view, 0, 0, -4.5);
  view_set_look_at (view, 0, 0, 0);

  light_set_pos (lights, 0, flame_pos[0], flame_pos[1], flame_pos[2]);

  view_fix (view, lights);
  light_fix (view, lights);

  vec_transform3_fipr (&tmp[0], &(*view->camera)[0][0], &flame_pos[0]);
  tmp[3] = 1.0f;
  vec_transform_fipr (&flame_pos_xformed[0], &(*view->projection)[0][0],
		      &tmp[0]);
  invw = 1.0 / flame_pos_xformed[3];
  flame_pos_xformed[0] *= invw;
  flame_pos_xformed[1] *= invw;
  flame_pos_xformed[2] = invw;
  
  /* Render to texture.  */
  pvr_wait_ready ();
  pvr_scene_begin_txr (warp_texture[warp_active], &tx_x, &tx_y);

#if 0
  user_clip.d1 = 0;  /* minx.  */
  user_clip.d2 = 7;  /* miny.  */
  user_clip.d3 = 1;  /* maxx.  */
  user_clip.d4 = 9;  /* maxy.  */
#endif

  pvr_list_begin (PVR_LIST_OP_POLY);
  //pvr_prim (&user_clip, sizeof (pvr_poly_hdr_t));
  draw_box ();
  pvr_list_finish ();

  pvr_list_begin (PVR_LIST_TR_POLY);
  //pvr_prim (&user_clip, sizeof (pvr_poly_hdr_t));
  draw_cooler ();
  pvr_list_finish ();

  pvr_scene_finish ();
}


static void
render_fire (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	     lighting *lights, int pass)
{
  if (pass == PVR_LIST_OP_POLY)
    draw_backdrop ();
  if (pass == PVR_LIST_TR_POLY)
    {
      draw_texture ();

      warp_active = 1 - warp_active;
    }
}

effect_methods fire_methods = {
  .preinit_assets = &preinit_fire,
  .init_effect = &init_effect,
  .prepare_frame = &do_feedback,
  .display_effect = &render_fire,
  .uninit_effect = &uninit_effect,
  .finalize = &finalize_effect
};
