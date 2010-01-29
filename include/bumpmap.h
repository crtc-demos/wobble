#ifndef BUMPMAP_H
#define BUMPMAP_H 1

#include <math.h>
#include <kos.h>

typedef struct
{
  /* Bump map intensity, 0.0 - 1.0.  */
  float intensity;
} bumpmap_info;

struct object;

/* This function is stolen from Fredrik Ehnbom (and modified a bit).

   T and Q defines the light source as polar coordinates where
   T is the elevation angle ranging from 0 to Pi/2 (90 degrees) and
   Q is the rotation angle ranging from 0 to Pi*2 (360 degrees).
   h is an intensity value specifying how much bumping should be done.
     0 <= h <= 1.
*/

static __inline__ unsigned int
bumpmap_set_bump_direction (float T, float Q, float h)
{
  unsigned int Q2 = ((unsigned int) ((Q / (2 * M_PI)) * 255)) & 255;
  unsigned int h2 = ((unsigned int) (h * 255)) & 255;

  unsigned int k1 = 255 - h2;
  unsigned int k2 = ((unsigned int) (h2 * fsin(T))) & 255;
  unsigned int k3 = ((unsigned int) (h2 * fcos(T))) & 255;

  return (k1 << 24) | (k2 << 16) | (k3 << 8) | Q2;
}

extern void bumpmap_auto_uv_orient (struct object *obj);
extern pvr_ptr_t bumpmap_load_raw (const char *fname, unsigned int xsize,
				   unsigned int ysize);

#endif
