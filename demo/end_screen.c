#include <png/png.h>

#include "object.h"
#include "timing.h"

static pvr_ptr_t end_screen;

static void
load_end_screen (void *params)
{
  end_screen = pvr_mem_malloc (1024 * 512 * 2);
  png_to_texture ("/rd/end-screen.png", end_screen, PNG_NO_ALPHA);
}

static void
free_end_screen (void *params)
{
  pvr_mem_free (end_screen);
}

static void
display_end_screen (uint32_t time_offset, void *params, int iparam,
		    viewpoint *view, lighting *lights, int pass)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;
  float rot, amp;
  float secs = (float) time_offset / 1000.0f;
  float corners[4][2] = {
    { -320, -240 },
    {  320, -240 },
    { -320,  240 },
    {  320,  240 }
  };
  int i;

  if (pass != PVR_LIST_OP_POLY)
    return;
  
  if (secs < 6) 
   return;
  
  if (secs < 8)
    {
      secs -= 6;
      amp = secs / 2.0f;
      rot = 4 * M_PI * (2.0f - secs);
    }
  else
    {
      amp = 1.0f;
      rot = 0.0f;
    }
  
  for (i = 0; i < 4; i++)
    {
      float tx, ty;
      
      tx = amp * (fcos (rot) * corners[i][0] - fsin (rot) * corners[i][1]);
      ty = amp * (fsin (rot) * corners[i][0] + fcos (rot) * corners[i][1]);
      
      corners[i][0] = 320 + tx;
      corners[i][1] = 240 + ty;
    }
  
  pvr_poly_cxt_txr (&cxt, pass, PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
		    1024, 512, end_screen, PVR_FILTER_BILINEAR);

  pvr_poly_compile (&hdr, &cxt);

  pvr_prim (&hdr, sizeof (hdr));

  vert.flags = PVR_CMD_VERTEX;
  vert.x = corners[0][0];
  vert.y = corners[0][1];
  vert.z = 0.001;
  vert.u = 0.0;
  vert.v = 0.0;
  vert.oargb = 0;
  vert.argb = 0xffffff;
  pvr_prim (&vert, sizeof (vert));

  vert.x = corners[1][0];
  vert.y = corners[1][1];
  vert.u = 640.0 / 1024.0;
  pvr_prim (&vert, sizeof (vert));

  vert.x = corners[2][0];
  vert.y = corners[2][1];
  vert.u = 0.0;
  vert.v = 480.0 / 512.0;
  pvr_prim (&vert, sizeof (vert));

  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = corners[3][0];
  vert.y = corners[3][1];
  vert.u = 640.0 / 1024.0;
  pvr_prim (&vert, sizeof (vert));
}

effect_methods end_screen_methods = {
  .preinit_assets = NULL,
  .init_effect = &load_end_screen,
  .prepare_frame = NULL,
  .display_effect = &display_end_screen,
  .uninit_effect = &free_end_screen,
  .finalize = NULL
};
