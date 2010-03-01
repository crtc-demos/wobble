#include <kos.h>
#include <GL/gl.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  int x;
  int y;
  unsigned int c;
} data_point;

static data_point duck[] = {
  { 584, 154, 0x171238 },
  { 608, 110, 0x70526 },
  { 314, 155, 0xc3a530 },
  { 296, 260, 0x2c238a },
  { 540, 221, 0xf7c604 },
  { 269, 233, 0xf2c40a },
  { 208, 393, 0x100d4e },
  { 503, 212, 0xbea234 },
  { 190, 454, 0x90730 },
  { 302, 296, 0x1d1794 },
  { 108, 255, 0x90730 },
  { 89, 329, 0x40318 },
  { 523, 6, 0x1a1425 },
  { 268, 345, 0x28206d },
  { 229, 401, 0xf0c51 },
  { 168, 313, 0xb79316 },
  { 495, 221, 0x4f4164 },
  { 595, 147, 0x90731 },
  { 21, 225, 0x4 },
  { 165, 294, 0xf1c103 },
  { 601, 214, 0xb093d },
  { 301, 376, 0x14106c },
  { 265, 377, 0x130f66 },
  { 136, 366, 0x8072e },
  { 547, 159, 0xfbc901 },
  { 224, 0, 0x141023 },
  { 559, 30, 0x60521 },
  { 635, 366, 0x70525 },
  { 60, 413, 0x10107 },
  { 58, 46, 0x1 },
  { 624, 184, 0x9072e },
  { 268, 255, 0xe4b70d },
  { 110, 266, 0xa88611 },
  { 461, 111, 0xe6bc14 },
  { 177, 312, 0xebbe0c },
  { 599, 189, 0xe0b3f },
  { 506, 299, 0x161173 },
  { 364, 16, 0xa0831 },
  { 160, 69, 0x70622 },
  { 155, 219, 0x181348 },
  { 509, 86, 0xf2c102 },
  { 584, 6, 0x10109 },
  { 550, 199, 0xf2c103 },
  { 134, 219, 0x90732 },
  { 578, 258, 0xd0a46 },
  { 299, 133, 0xf0c20b },
  { 576, 159, 0x685327 },
  { 590, 355, 0xd0a43 },
  { 176, 286, 0xefbf04 },
  { 221, 246, 0xf6c502 },
  { 413, 173, 0xb99f37 },
  { 447, 181, 0xb59b39 },
  { 156, 368, 0x14103d },
  { 215, 295, 0xfecb00 },
  { 289, 97, 0xf9c802 },
  { 416, 225, 0x1e1898 },
  { 566, 251, 0x705933 },
  { 130, 123, 0x70526 },
  { 364, 249, 0x2019a4 },
  { 600, 87, 0x6051f },
  { 561, 162, 0xfecb00 },
  { 369, 146, 0xbca136 },
  { 364, 25, 0xeebf05 },
  { 64, 239, 0x40318 }
};

static data_point scream[] = {
  { 454, 363, 0xfbda00 },
  { 533, 302, 0x0 },
  { 348, 42, 0x1d1900 },
  { 546, 183, 0x0 },
  { 165, 176, 0x504500 },
  { 88, 336, 0x10100 },
  { 91, 299, 0x0 },
  { 504, 62, 0x20200 },
  { 217, 228, 0x0 },
  { 492, 263, 0x0 },
  { 471, 321, 0xeecf00 },
  { 457, 182, 0xd0b00 },
  { 370, 21, 0x20100 },
  { 250, 246, 0x50500 },
  { 217, 115, 0xfcdb00 },
  { 128, 178, 0x10000 },
  { 213, 377, 0x10000 },
  { 569, 87, 0x0 },
  { 266, 384, 0x120f00 },
  { 204, 78, 0x60500 },
  { 416, 201, 0x0 },
  { 473, 404, 0x20100 },
  { 432, 411, 0x0 },
  { 166, 209, 0x60500 },
  { 247, 190, 0x60500 },
  { 371, 167, 0x0 },
  { 128, 93, 0x30300 },
  { 219, 365, 0xf0d100 },
  { 213, 129, 0xf6d600 },
  { 535, 114, 0x1c1900 },
  { 489, 15, 0x0 },
  { 120, 200, 0x0 },
  { 315, 24, 0x50400 },
  { 475, 161, 0xf3d400 },
  { 500, 202, 0x0 },
  { 415, 258, 0x100e00 },
  { 239, 205, 0x0 },
  { 478, 118, 0xe8ca00 },
  { 230, 308, 0xf2d300 },
  { 428, 330, 0xefd000 },
  { 253, 338, 0xefd000 },
  { 571, 117, 0x0 },
  { 183, 173, 0xe9cb00 },
  { 377, 99, 0xefd000 },
  { 94, 25, 0x0 },
  { 173, 238, 0x50400 },
  { 131, 13, 0x0 },
  { 316, 317, 0x30300 },
  { 278, 414, 0x0 },
  { 243, 345, 0xffde00 },
  { 538, 51, 0x0 },
  { 540, 316, 0x0 },
  { 69, 145, 0x0 },
  { 346, 86, 0xf1d200 },
  { 441, 316, 0xfddc00 },
  { 333, 388, 0x0 },
  { 534, 185, 0x10100 },
  { 487, 129, 0xf9d900 },
  { 343, 188, 0x0 },
  { 244, 393, 0x10100 },
  { 201, 241, 0x100d00 },
  { 87, 343, 0x0 },
  { 552, 368, 0x0 },
  { 183, 158, 0xfedd00 }
};

static data_point letter_c1[] = {
  { 170, 330, 0x4f5a13 },
  { 442, 156, 0xe074bc },
  { 552, 192, 0x4e5611 },
  { 550, 63, 0x585e0f },
  { 557, 478, 0x2a401e },
  { 264, 190, 0xd65fae },
  { 431, 463, 0x65680d },
  { 337, 159, 0xcd65a6 },
  { 547, 0, 0x545c10 },
  { 306, 365, 0xc05ea0 },
  { 611, 219, 0x5e620d },
  { 48, 264, 0x32451b },
  { 477, 275, 0x485115 },
  { 46, 221, 0x33451b },
  { 241, 102, 0x2f431c },
  { 418, 212, 0x3c4a1c },
  { 324, 150, 0xb94d96 },
  { 262, 231, 0xc15998 },
  { 507, 347, 0x54590f },
  { 423, 402, 0xde67b4 },
  { 619, 342, 0x475516 },
  { 189, 400, 0x384b1b },
  { 0, 174, 0x3c4b17 },
  { 242, 130, 0x403931 },
  { 246, 296, 0xc25999 },
  { 433, 358, 0x763952 },
  { 551, 129, 0x555911 },
  { 196, 80, 0x35471a },
  { 104, 402, 0x2e431c },
  { 455, 102, 0x653245 },
  { 485, 465, 0x505c14 },
  { 323, 375, 0xcd63aa },
  { 462, 163, 0xd261ae },
  { 108, 47, 0x3f4d16 },
  { 321, 258, 0x4a4428 },
  { 168, 359, 0x475517 },
  { 6, 84, 0x465114 },
  { 104, 362, 0x2b411e },
  { 329, 301, 0x484720 },
  { 367, 233, 0x32451b },
  { 511, 438, 0x596011 },
  { 179, 280, 0x4c5715 },
  { 636, 326, 0x5e6611 },
  { 424, 260, 0x3d4c17 },
  { 127, 39, 0x3d4b17 },
  { 242, 439, 0x445317 },
  { 282, 93, 0x3a451e },
  { 361, 194, 0x33451e },
  { 68, 138, 0x35471a },
  { 583, 255, 0x555b0f },
  { 519, 162, 0x6d304c },
  { 4, 98, 0x404e16 },
  { 231, 392, 0x5d610d },
  { 401, 308, 0x464a1c },
  { 52, 340, 0x35471a },
  { 205, 154, 0x374a1b },
  { 75, 404, 0x394919 },
  { 427, 37, 0x414d17 },
  { 299, 462, 0x656710 },
  { 459, 67, 0x475214 },
  { 255, 374, 0xbf668d },
  { 285, 448, 0x5b610f },
  { 217, 385, 0x5c600d },
  { 353, 157, 0xb64f93 }
};

static data_point letter_r[] = {
  { 252, 382, 0xe0f40a },
  { 521, 71, 0xc00ff },
  { 231, 444, 0x1005f9 },
  { 1, 282, 0xc00ff },
  { 242, 143, 0xe2f608 },
  { 288, 71, 0xe3f707 },
  { 253, 149, 0xeaff00 },
  { 474, 257, 0x1b12ec },
  { 57, 330, 0xc00ff },
  { 440, 245, 0xe3f806 },
  { 466, 438, 0xf03fb },
  { 524, 322, 0xc00ff },
  { 418, 379, 0xddf00e },
  { 256, 51, 0xcdde20 },
  { 277, 336, 0xc00fe },
  { 146, 453, 0xc00ff },
  { 348, 469, 0x6a6c92 },
  { 378, 61, 0x1005f9 },
  { 373, 131, 0x1c13eb },
  { 334, 329, 0x2a23db },
  { 482, 65, 0xc00ff },
  { 463, 182, 0xe3f707 },
  { 601, 115, 0x1207f7 },
  { 146, 110, 0xc01fd },
  { 67, 310, 0xc00fe },
  { 269, 334, 0xd01fd },
  { 481, 260, 0xc00ff },
  { 455, 260, 0x2f28d6 },
  { 581, 264, 0xc00ff },
  { 488, 364, 0xe02fc },
  { 389, 390, 0xe9fe00 },
  { 174, 290, 0xe9fe00 },
  { 315, 102, 0xe6fb03 },
  { 425, 94, 0xe02fc },
  { 575, 229, 0xc00fe },
  { 240, 419, 0xdff30b },
  { 337, 2, 0x1207f7 },
  { 91, 250, 0xf03fb },
  { 377, 327, 0xe7fb03 },
  { 150, 303, 0xe6fb03 },
  { 178, 349, 0xe2f608 },
  { 46, 52, 0xc00ff },
  { 191, 18, 0xc00ff },
  { 57, 118, 0xc00ff },
  { 267, 1, 0x413dc1 },
  { 361, 272, 0xe8fc02 },
  { 326, 413, 0xe3f707 },
  { 518, 98, 0x7a7f7f },
  { 81, 410, 0xe02fc },
  { 477, 244, 0x979f5f },
  { 580, 77, 0xc00ff },
  { 243, 379, 0xe8fd01 },
  { 243, 268, 0x6f718d },
  { 526, 155, 0xe6fb03 },
  { 292, 79, 0xeaff00 },
  { 388, 255, 0xe2f509 },
  { 242, 236, 0xe8fd01 },
  { 104, 329, 0xccdd21 },
  { 377, 24, 0xc00ff },
  { 620, 393, 0xc00ff },
  { 318, 217, 0xe3f707 },
  { 357, 58, 0xa5b04e },
  { 322, 333, 0xd01fd },
  { 400, 110, 0xc00ff }
};

static data_point letter_t[] = {
  { 552, 300, 0xa01d1e },
  { 5, 334, 0x8b1415 },
  { 9, 179, 0x8a1415 },
  { 108, 404, 0x8d1719 },
  { 137, 143, 0xdc3737 },
  { 598, 411, 0x891314 },
  { 31, 231, 0x7e0e10 },
  { 624, 185, 0x9d1b1d },
  { 402, 314, 0x975a5b },
  { 293, 7, 0x71090b },
  { 203, 382, 0xcccccc },
  { 198, 174, 0xcf3131 },
  { 213, 418, 0xcdcdcd },
  { 243, 132, 0x8f1b1d },
  { 228, 68, 0xa51f20 },
  { 452, 145, 0xda3636 },
  { 592, 330, 0x991a1b },
  { 546, 104, 0xd83535 },
  { 546, 279, 0xc02a2b },
  { 316, 313, 0xcccccc },
  { 216, 264, 0x9e2224 },
  { 425, 291, 0x8f191a },
  { 509, 88, 0xb92829 },
  { 409, 363, 0xcccccc },
  { 49, 235, 0xbf2a2b },
  { 486, 193, 0xdd3737 },
  { 177, 401, 0xcccccc },
  { 630, 101, 0x7d0e10 },
  { 309, 92, 0xc8c4c4 },
  { 289, 249, 0xcccbcb },
  { 213, 419, 0x7c2425 },
  { 502, 103, 0xe2393a },
  { 400, 94, 0x9a1e20 },
  { 459, 343, 0xcdcdcd },
  { 423, 372, 0xcbcaca },
  { 308, 160, 0xc8c4c4 },
  { 315, 2, 0x73090b },
  { 480, 0, 0x7d0e10 },
  { 494, 279, 0x951a1b },
  { 632, 211, 0x8a1415 },
  { 460, 324, 0x965d5e },
  { 461, 274, 0xb12425 },
  { 159, 43, 0xaa2122 },
  { 246, 251, 0x7f1214 },
  { 431, 466, 0x760c0e },
  { 564, 101, 0xaa2122 },
  { 628, 293, 0x811011 },
  { 418, 165, 0x9a1c1d },
  { 335, 469, 0x7b181a },
  { 215, 399, 0xcdcdcd },
  { 470, 336, 0xcbc9c9 },
  { 497, 470, 0x770b0d },
  { 210, 185, 0xa41f20 },
  { 213, 467, 0x760b0d },
  { 252, 286, 0x8e4344 },
  { 304, 364, 0xcccbcb },
  { 547, 374, 0x922425 },
  { 130, 0, 0x841113 },
  { 170, 442, 0x781011 },
  { 511, 299, 0x914f51 },
  { 257, 35, 0x801f21 },
  { 298, 138, 0xcccccc },
  { 536, 451, 0x811213 },
  { 471, 377, 0xc5bfbf }
};

static data_point letter_c2[] = {
  { 267, 10, 0x4a0cf7 },
  { 328, 165, 0x4403ff },
  { 56, 67, 0x4403ff },
  { 336, 393, 0xeafb40 },
  { 630, 18, 0x4403ff },
  { 446, 471, 0x4403fe },
  { 630, 119, 0x4403ff },
  { 119, 383, 0x4606fc },
  { 174, 398, 0x490bf8 },
  { 254, 400, 0xe7f644 },
  { 391, 248, 0x4403ff },
  { 247, 15, 0x4404fd },
  { 203, 372, 0xe7f644 },
  { 358, 307, 0x4a0cf7 },
  { 495, 342, 0x4403fe },
  { 248, 443, 0x4403fe },
  { 433, 135, 0xdee94e },
  { 176, 238, 0xe5f446 },
  { 332, 61, 0xeafb40 },
  { 537, 52, 0x4403ff },
  { 191, 149, 0xecfd3f },
  { 262, 448, 0x4403ff },
  { 93, 270, 0x4e12f2 },
  { 113, 63, 0x4403ff },
  { 487, 164, 0x4a0df7 },
  { 502, 258, 0x4403ff },
  { 249, 191, 0x4a0cf7 },
  { 444, 204, 0x4504fd },
  { 383, 200, 0x4403fe },
  { 439, 23, 0x4403ff },
  { 296, 79, 0xedff3e },
  { 260, 436, 0x9d8898 },
  { 325, 3, 0x4504fd },
  { 306, 178, 0x4403ff },
  { 362, 1, 0x4b0df6 },
  { 513, 85, 0x4606fc },
  { 331, 117, 0xe9fa41 },
  { 374, 450, 0x4d11f3 },
  { 75, 184, 0x4606fc },
  { 113, 116, 0x4f14f1 },
  { 239, 456, 0x4403ff },
  { 109, 86, 0x4403fe },
  { 150, 453, 0x4403ff },
  { 291, 295, 0x4403fe },
  { 401, 100, 0xecfe3e },
  { 57, 334, 0x4403ff },
  { 84, 232, 0x4403ff },
  { 448, 47, 0x4403fe },
  { 414, 95, 0xe0ec4c },
  { 370, 381, 0xe7f644 },
  { 505, 165, 0x4403ff },
  { 431, 25, 0x4606fc },
  { 198, 17, 0x4708fa },
  { 322, 300, 0x4403fe },
  { 65, 146, 0x4403ff },
  { 66, 230, 0x4403ff },
  { 276, 395, 0xe9f942 },
  { 263, 90, 0xe5f446 },
  { 327, 146, 0x561eea },
  { 318, 160, 0x4707fb },
  { 351, 216, 0x4403ff },
  { 237, 227, 0x4809f9 },
  { 344, 469, 0x4403fe },
  { 499, 390, 0x4404fe }
};

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

#define NUMBER 64

static float xpos[NUMBER], ypos[NUMBER];
static float dx[NUMBER], dy[NUMBER];
static float pt_red[NUMBER], pt_green[NUMBER], pt_blue[NUMBER];

#define CLAMPING 0

static float clamp (float val, float max)
{
#if CLAMPING
  if (val > max)
    return max;
  else if (val < -max)
    return -max;
  else
    return val;
#else
  return val;
#endif
}

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
  float phase = 0, xmag = 800, xfreq = 0.5, ymag = 800, yfreq = 0.7;
  int shape = 0;
  
  cable_type = vid_check_cable ();
  if (cable_type == CT_VGA)
    vid_init (DM_640x480_VGA, PM_RGB565);
  else
    vid_init (DM_640x480_NTSC_IL, PM_RGB565);
  
  for (i = 0; i < NUMBER; i++)
    {
      /*float r = (float)(rand () & 255) / 255.0f;
      float g = (float)(rand () & 255) / 255.0f;
      float b = (float)(rand () & 255) / 255.0f;*/
      xpos[i] = rand () % 640;
      ypos[i] = rand () % 480;
      dx[i] = ((rand () & 255) - 127.5) / 32.0f;
      dy[i] = ((rand () & 255) - 127.5) / 32.0f;
      /* colour[i] = PVR_PACK_COLOR (1.0f, r, g, b); */
    }
  
  pvr_init (&params);

  glKosInit ();
  
  while (!quit)
    {
      MAPLE_FOREACH_BEGIN (MAPLE_FUNC_CONTROLLER, cont_state_t, st)
        if (st->buttons & CONT_START)
	  quit = 1;
	if (st->buttons & CONT_DPAD_UP)
	  shape = 0;
	if (st->buttons & CONT_DPAD_LEFT)
	  shape = 1;
	if (st->buttons & CONT_DPAD_DOWN)
	  shape = 2;
	if (st->buttons & CONT_DPAD_RIGHT)
	  shape = 3;
	if (st->buttons & CONT_A)
	  shape = 4;
	if (st->buttons & CONT_B)
	  shape = 5;
      MAPLE_FOREACH_END ()
      
      glKosBeginFrame ();
      
      for (i = 0; i < NUMBER; i++)
        {
	  /*float r = ypos[i] / 240.0f;
	  float g = ypos[i] / 480.0f;
	  float b = (320 - abs (xpos[i] - 320)) / 320.0f - 1.5 + g;*/
	  float want_x, want_y, have_x, have_y;
	  float want_angle, have_angle, want_magn, have_magn;
	  float want_red, want_green, want_blue;
	  float angle_diff, magn_diff, red_diff, green_diff, blue_diff;
	  uint32_t col;
	  uint32_t want_colour;
	  int ir, ig, ib;
	  	  
	  switch (shape)
	    {
	    case 0:
	      want_x = letter_c1[i].x;
	      want_y = 479 - letter_c1[i].y;
	      want_colour = letter_c1[i].c;
	      break;
	    case 1:
	      want_x = letter_r[i].x;
	      want_y = 479 - letter_r[i].y;
	      want_colour = letter_r[i].c;
	      break;
	    case 2:
	      want_x = letter_t[i].x;
	      want_y = 479 - letter_t[i].y;
	      want_colour = letter_t[i].c;
	      break;
	    case 3:
	      want_x = letter_c2[i].x;
	      want_y = 479 - letter_c2[i].y;
	      want_colour = letter_c2[i].c;
	      break;
	    case 4:
	      want_x = duck[i].x;
	      want_y = 479 - duck[i].y;
	      want_colour = duck[i].c;
	      break;
	    case 5:
	      want_x = scream[i].x;
	      want_y = 479 - scream[i].y;
	      want_colour = scream[i].c;
	      break;
	    default:
	      want_x = 320;
	      want_y = 240;
	      want_colour = 0;
	    }

	  want_red = ((float) ((want_colour >> 16) & 255)) / 255.0f;
	  want_green = ((float) ((want_colour >> 8) & 255)) / 255.0f;
	  want_blue = ((float) (want_colour & 255)) / 255.0f;

          want_x -= 320.0f;
	  want_y -= 240.0f;

	  have_x = xpos[i] - 320.0;
	  have_y = ypos[i] - 240.0;

          want_angle = atan2 (want_y, want_x);
	  have_angle = atan2 (have_y, have_x);
	  
	  if ((have_angle - want_angle) > M_PI)
	    have_angle -= 2 * M_PI;
	  if ((have_angle - want_angle) < -M_PI)
	    have_angle += 2 * M_PI;
	  
	  have_magn = sqrtf (have_x * have_x + have_y * have_y);
	  want_magn = sqrtf (want_x * want_x + want_y * want_y);
	  
	  angle_diff = (want_angle - have_angle) / 32.0f;
	  have_angle += clamp (angle_diff, 0.02);
	  magn_diff = (want_magn - have_magn) / 32.0f;
	  have_magn += clamp (magn_diff, 5);

	  xpos[i] = 320.0 + have_magn * cosf (have_angle);
	  ypos[i] = 240.0 + have_magn * sinf (have_angle);
	  red_diff = (want_red - pt_red[i]) / 32.0f;
	  pt_red[i] += clamp (red_diff, 0.005);
	  green_diff = (want_green - pt_green[i]) / 32.0f;
	  pt_green[i] += clamp (green_diff, 0.005);
	  blue_diff = (want_blue - pt_blue[i]) / 32.0f;
	  pt_blue[i] += clamp (blue_diff, 0.005);
	  
	  ir = pt_red[i] * 255.0f;
	  ig = pt_green[i] * 255.0f;
	  ib = pt_blue[i] * 255.0f;
	  
	  ir = ir > 255 ? 255 : ir < 0 ? 0 : ir;
	  ig = ig > 255 ? 255 : ig < 0 ? 0 : ig;
	  ib = ib > 255 ? 255 : ib < 0 ? 0 : ib;
	  
	  col = (ir << 16) | (ig << 8) | ib;
	  
          cone (xpos[i] + fsin (phase * dx[i]) * 2,
	        ypos[i] + fcos (phase * dy[i]) * 2, 200, col);
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
