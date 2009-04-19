/* Feedback test.  */

#include <math.h>
#include <stdlib.h>

#include <kos.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <png/png.h>

#include "perlin.h"

extern uint8 romdisk[];

KOS_INIT_FLAGS (INIT_DEFAULT);
KOS_INIT_ROMDISK (romdisk);

static void
init_pvr (void)
{
  pvr_init_params_t params = {
    { PVR_BINSIZE_16,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_16,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_0 },	/* Punch-thrus.  */
    512 * 1024,		/* Vertex buffer size 512K.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  
  pvr_init (&params);
}

float rot1 = 0.0;
float perlin_phase = 0.0;
float cooling_offset = 0.0;
pvr_ptr_t warp_texture[2];
pvr_ptr_t cooling_texture;
int warp_active = 0;

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

void
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
      vert.z = 5.0f;
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
  vert.y = 120 + fsin (rot1) * 50;
  vert.z = 6.0f;
  vert.u = vert.v = 0.0f;
  vert.argb = 0xffff9944;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 320 + fcos (rot1 + M_PI / 2.0) * 50;
  vert.y = 120 + fsin (rot1 + M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.x = 320 + fcos (rot1 + 3.0 * M_PI / 2.0) * 50;
  vert.y = 120 + fsin (rot1 + 3.0 * M_PI / 2.0) * 50;
  pvr_prim (&vert, sizeof (vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = 320 + fcos (rot1 + M_PI) * 50;
  vert.y = 120 + fsin (rot1 + M_PI) * 50;
  pvr_prim (&vert, sizeof (vert));

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

  rot1 += 0.05;
  if (rot1 > 2 * M_PI)
    rot1 -= 2 * M_PI;
  
  perlin_phase += 0.03;
}

void
draw_texture (void)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t poly;
  pvr_vertex_t vert;
  
  pvr_poly_cxt_txr (&cxt, PVR_LIST_OP_POLY,
		    PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, 1024, 512,
		    warp_texture[1 - warp_active], PVR_FILTER_NONE);
  pvr_poly_compile (&poly, &cxt);
  
  pvr_prim (&poly, sizeof (poly));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = 0.0f;
  vert.y = 480.0f;
  vert.z = 5.0f;
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

void
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

int
main (int argc, char *argv[])
{
  int cable_type;
  int quit = 0;
  uint32 tx_x = 1024, tx_y = 512;

  cable_type = vid_check_cable ();
  
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_PAL_IL, PM_RGB565);
  
  init_pvr ();
  
  warp_texture[0] = pvr_mem_malloc (1024 * 512 * 2);
  warp_texture[1] = pvr_mem_malloc (1024 * 512 * 2);
  cooling_texture = pvr_mem_malloc (1024 * 512 * 2);
  
  render_cooling_texture ();
  
  vid_border_color (0, 0, 0);
  pvr_set_bg_color (0.0, 0.0, 0.0);

  while (!quit)
    {
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()
      
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
      
      /* Now render texture to screen.  */
      pvr_wait_ready ();
      pvr_scene_begin ();
      
      pvr_list_begin (PVR_LIST_OP_POLY);
      draw_texture ();
      pvr_list_finish ();
      
      pvr_scene_finish ();
      
      warp_active = 1 - warp_active;
    }

  pvr_mem_free (cooling_texture);
  pvr_mem_free (warp_texture[1]);
  pvr_mem_free (warp_texture[0]);
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
