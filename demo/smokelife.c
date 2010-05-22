#include <stdlib.h>

#include <kos.h>
#include <kmg/kmg.h>

#include "object.h"
#include "timing.h"

static pvr_ptr_t sltex;
static pvr_ptr_t blob;
static pvr_ptr_t dub;

#define SMOKES 8

static float xpos[SMOKES], ypos[SMOKES];
static float rot[SMOKES], rspd[SMOKES];

void
smokelife_preinit (void)
{
  static int initialised = 0;
  kos_img_t txr;
  unsigned int i;
  
  if (initialised)
    return;

  kmg_to_img ("/rd/smokelife.kmg", &txr);
  sltex = pvr_mem_malloc (512 * 128 * 2);
  pvr_txr_load_kimg (&txr, sltex, PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);

  kmg_to_img ("/rd/smoke_blob.kmg", &txr);
  blob = pvr_mem_malloc (256 * 256 * 2);
  pvr_txr_load_kimg (&txr, blob, PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);

  kmg_to_img ("/rd/dub.kmg", &txr);
  dub = pvr_mem_malloc (256 * 128 * 2);
  pvr_txr_load_kimg (&txr, dub, PVR_TXRLOAD_SQ);
  kos_img_free (&txr, 0);
  
  for (i = 0; i < SMOKES; i++)
    {
      xpos[i] = rand () % 640;
      ypos[i] = rand () % 480;
      rot[i] = 2 * M_PI * (rand () % 1024) / 1024.0;
      rspd[i] = ((rand () % 256) - 128) / 4096.0;
    }
  
  initialised = 1;
}

static void
finalize_smokes (void *params)
{
  pvr_mem_free (blob);
  pvr_mem_free (sltex);
  pvr_mem_free (dub);
}

static void
prepare_frame (uint32_t time_offset, void *params, int iparam,
	       viewpoint *view, lighting *lights)
{
  unsigned int i;
  
  for (i = 0; i < SMOKES; i++)
    {
      rot[i] += rspd[i];
    }
}

static void
smokelife_effect (uint32_t time_offset, void *params, int iparam,
		  viewpoint *view, lighting *lights, int pass)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_poly_hdr_t hdr2;
  pvr_poly_hdr_t hdr3;
  pvr_vertex_t vert;
  int i, num;
  float amp = audio_amplitude (33);
  
  if (pass != PVR_LIST_OP_POLY && pass != PVR_LIST_TR_POLY)
    return;

  pvr_poly_cxt_txr (&cxt, pass, PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
		    512, 128, sltex, PVR_FILTER_BILINEAR);

  if (pass == PVR_LIST_TR_POLY)
    {
      cxt.blend.src = PVR_BLEND_ONE;
      cxt.blend.dst = PVR_BLEND_ONE;
      cxt.txr.env = PVR_TXRENV_MODULATE;
      cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
      vert.argb = 0xff101010;
      num = 16;
    }
  else
    {
      vert.argb = 0xffffffff;
      num = 1;
    }

  pvr_poly_compile (&hdr, &cxt);
  
  pvr_prim (&hdr, sizeof (hdr));

  for (i = 0; i < num; i++)
    {
      float y_size = 80 + (i * amp);

      vert.flags = PVR_CMD_VERTEX;
      vert.x = 0;
      vert.y = 240 - y_size;
      vert.z = 5 + (float) i;
      vert.u = 0.0;
      vert.v = 0.0;
      vert.oargb = 0;
      pvr_prim (&vert, sizeof (vert));

      vert.x = 640;
      vert.u = 1.0;
      pvr_prim (&vert, sizeof (vert));

      vert.x = 0;
      vert.y = 240 + y_size;
      vert.u = 0.0;
      vert.v = 1.0;
      pvr_prim (&vert, sizeof (vert));

      vert.flags = PVR_CMD_VERTEX_EOL;
      vert.x = 640;
      vert.u = 1.0;
      pvr_prim (&vert, sizeof (vert));
    }
  
  if (pass == PVR_LIST_TR_POLY)
    {
      pvr_poly_cxt_txr (&cxt, pass, PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
			256, 256, blob, PVR_FILTER_BILINEAR);
      cxt.blend.src = PVR_BLEND_ONE;
      cxt.blend.dst = PVR_BLEND_ONE;
      cxt.txr.env = PVR_TXRENV_MODULATE;
      cxt.depth.comparison = PVR_DEPTHCMP_GEQUAL;
      vert.argb = 0xff404040;
      pvr_poly_compile (&hdr2, &cxt);

      pvr_prim (&hdr2, sizeof (hdr2));

      for (i = 0; i < SMOKES; i++)
        {
	  float this_xpos = xpos[i];
	  float this_ypos = ypos[i];
	  float rotate = rot[i];
	  const float blobsize = 200.0;

	  vert.flags = PVR_CMD_VERTEX;
	  vert.x = this_xpos + blobsize * fcos (rotate);
	  vert.y = this_ypos + blobsize * fsin (rotate);
	  vert.z = 6 - (i / (float) SMOKES);
	  vert.u = 0.0;
	  vert.v = 0.0;
	  vert.oargb = 0;
	  pvr_prim (&vert, sizeof (vert));

	  vert.x = this_xpos + blobsize * fcos (rotate + M_PI / 2.0);
	  vert.y = this_ypos + blobsize * fsin (rotate + M_PI / 2.0);
	  vert.u = 1.0;
	  pvr_prim (&vert, sizeof (vert));

	  vert.x = this_xpos + blobsize * fcos (rotate + 3.0 * M_PI / 2.0);
	  vert.y = this_ypos + blobsize * fsin (rotate + 3.0 * M_PI / 2.0);
	  vert.u = 0.0;
	  vert.v = 1.0;
	  pvr_prim (&vert, sizeof (vert));

	  vert.flags = PVR_CMD_VERTEX_EOL;
	  vert.x = this_xpos + blobsize * fcos (rotate + M_PI);
	  vert.y = this_ypos + blobsize * fsin (rotate + M_PI);
	  vert.u = 1.0;
	  pvr_prim (&vert, sizeof (vert));
        }
    }
  else if (pass == PVR_LIST_OP_POLY)
    {
      float proportion, size, secs = (float) time_offset / 1000.0f;
      const float lhs = -0.5;
      const float rhs = 1.3;
      const float mid = (lhs + rhs) / 2.0f;
      
      pvr_poly_cxt_txr (&cxt, pass, PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
			256, 128, dub, PVR_FILTER_BILINEAR);
      vert.argb = 0xffffffff;
      cxt.txr.uv_clamp = PVR_UVCLAMP_UV;
      pvr_poly_compile (&hdr3, &cxt);

      if (secs > 0.5f && secs <= 1.0f)
        proportion = (secs - 0.5) * 2.0f;
      else if (secs > 1.0f && secs <= 4.0f)
        proportion = 1.0f;
      else if (secs > 4.0f && secs <= 4.5f)
        proportion = (4.5f - secs) * 2.0f;
      else
        proportion = 0.0f;

      size = proportion * 200;

      pvr_prim (&hdr3, sizeof (hdr3));

      vert.flags = PVR_CMD_VERTEX;
      vert.x = 640.0f;
      vert.y = 480.0f - size;
      vert.z = 10.0f;
      vert.u = mid + proportion * (rhs - mid);
      vert.v = 0.0f;
      vert.oargb = 0;
      pvr_prim (&vert, sizeof (vert));

      vert.flags = PVR_CMD_VERTEX;
      vert.x = 640.0f;
      vert.y = 480.0f;
      vert.u = (lhs + rhs) / 2.0f;
      vert.v = proportion * 1.5;
      vert.oargb = 0;
      pvr_prim (&vert, sizeof (vert));

      vert.flags = PVR_CMD_VERTEX_EOL;
      vert.x = 640.0f - size;
      vert.y = 480.0f;
      vert.u = mid + proportion * (lhs - mid);
      vert.v = 0.0f;
      vert.oargb = 0;
      pvr_prim (&vert, sizeof (vert));
    }
}

effect_methods smokelife_methods = {
  .preinit_assets = &smokelife_preinit,
  .init_effect = NULL,
  .prepare_frame = &prepare_frame,
  .display_effect = &smokelife_effect,
  .uninit_effect = NULL,
  .finalize = &finalize_smokes
};
