#include <kos.h>
#include <GL/gl.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

void
cone (float cx, float cy, float radius, uint32_t colour)
{
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;
  int i;
  static float offx[30], offy[30];
  static int inited = 0;
  
  if (!inited)
    {
      for (i = 0; i < 30; i++)
        {
	  offx[i] = fcos ((float) i / 15.0f * M_PI) * radius;
	  offy[i] = fsin ((float) i / 15.0f * M_PI) * radius;
	}
      inited = 1;
    }
  
  pvr_poly_cxt_col (&cxt, PVR_LIST_OP_POLY);
  pvr_poly_compile (&hdr, &cxt);
  
  vert.oargb = 0;
  vert.argb = colour;
  vert.u = vert.v = 0;

  pvr_prim (&hdr, sizeof (hdr));
    
  for (i = 0; i < 30; i++)
    {
      int next = (i == 29) ? 0 : i + 1;
      
      vert.flags = PVR_CMD_VERTEX;
      vert.x = cx;
      vert.y = cy;
      vert.z = 20;
      
      pvr_prim (&vert, sizeof (vert));
      
      vert.x = cx + offx[i];
      vert.y = cy + offy[i];
      vert.z = 1;
      
      pvr_prim (&vert, sizeof (vert));
      
      vert.flags = PVR_CMD_VERTEX_EOL;
      vert.x = cx + offx[next];
      vert.y = cy + offy[next];
      vert.z = 1;
      
      pvr_prim (&vert, sizeof (vert));
    }
}

#define NUMBER 100

int
main (int argc, char *argv[])
{
  int cable_type, i;
  int quit = 0;
  pvr_init_params_t params = {
    { PVR_BINSIZE_32,	/* Opaque polygons.  */
      PVR_BINSIZE_0,	/* Opaque modifiers.  */
      PVR_BINSIZE_0,	/* Translucent polygons.  */
      PVR_BINSIZE_0,	/* Translucent modifiers.  */
      PVR_BINSIZE_0 },	/* Punch-thrus.  */
    512 * 1024,		/* Vertex buffer size 512K.  */
    0,			/* No DMA.  */
    0			/* No FSAA.  */
  };
  float xpos[NUMBER], ypos[NUMBER];
  float dx[NUMBER], dy[NUMBER];
  uint32_t colour[NUMBER];
  float phase = 0, xmag = 800, xfreq = 0.5, ymag = 800, yfreq = 0.7;
  
  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_NTSC_IL, PM_RGB565);
  
  for (i = 0; i < NUMBER; i++)
    {
      float r = (float)(rand () & 255) / 255.0f;
      float g = (float)(rand () & 255) / 255.0f;
      float b = (float)(rand () & 255) / 255.0f;
      xpos[i] = rand () % 640;
      ypos[i] = rand () % 480;
      dx[i] = ((rand () & 255) - 127.5) / 32.0f;
      dy[i] = ((rand () & 255) - 127.5) / 32.0f;
      colour[i] = PVR_PACK_COLOR (1.0f, r, g, b);
    }
  
  pvr_init (&params);

  glKosInit ();
  
  while (!quit)
    {
      float xbar = 320 + xmag * sin (xfreq * phase);
      float ybar = 240 + ymag * sin (yfreq * phase);
      
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
      MAPLE_FOREACH_END ()
      
      glKosBeginFrame ();
      
      for (i = 0; i < NUMBER; i++)
        {
	  float r = ypos[i] / 240.0f;
	  float g = ypos[i] / 480.0f;
	  float b = (320 - abs (xpos[i] - 320)) / 320.0f - 1.5 + g;
	  uint32_t col;
	  
	  if (r > 1.0f)
	    r = 1.0f;
	  if (g > 1.0f)
	    g = 1.0f;
	  if (b > 1.0f)
	    b = 1.0f;
	  if (r < 0.0f)
	    r = 0.0f;
	  if (g < 0.0f)
	    g = 0.0f;
	  if (b < 0.0f)
	    b = 0.0f;
	  
	  if ((xpos[i] > xbar - 40 && xpos[i] < xbar + 40)
	      || (ypos[i] > ybar - 40 && ypos[i] < ybar + 40))
	    r = g = b = 1.0f;
	  
	  col = PVR_PACK_COLOR (1.0f, r, g, b);
	  
          cone (xpos[i], ypos[i], 200, col);
	  xpos[i] += dx[i];
	  ypos[i] += dy[i];
	  if (xpos[i] >= 640.0f)
	    dx[i] = -dx[i];
	  if (ypos[i] >= 480.0f)
	    dy[i] = -dy[i];
	  if (xpos[i] < 0)
	    dx[i] = -dx[i];
	  if (ypos[i] < 0)
	    dy[i] = -dy[i];
	}
      
      //glKosFinishList ();
      glKosFinishFrame ();
      
      phase += 0.05;
    }
  
  glKosShutdown ();
  
  pvr_shutdown ();
  vid_shutdown ();
  
  return 0;
}
