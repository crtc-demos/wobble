#include "perlin.h"

static float hash(int x, int y)
{
  x += y*1471;
  x = (x<<13) ^ x;
  return (1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff)
          / 1073741824.0);
}

/* 2D smoothed noise weightings:

  1  2  1
  2  4  2
  1  2  1

*/

#if 0
static float smooth(int x, int y)
{
  return ((hash(x-1, y-1) + hash(x+1, y-1) + hash(x-1, y+1) + hash(x+1, y+1)) +
          (hash(x-1, y) + hash(x+1, y) + hash(x, y-1) + hash(x, y+1))*2.0 +
           hash(x,y)*4.0)/16.0;
}
#endif

static float cubic(float v0, float v1, float v2, float v3, float x)
{
  float p = (v3-v2) - (v0-v1);
  float q = (v0-v1) - p;
  float r = v2 - v0;
  float s = v1;
  s += r*x;
  s += q*x*x;
  s += p*x*x*x;
  return s;
}

static float cubicxy(float x, float y)
{
  int ix = (int)x, iy = (int)y;
  float fx = x-ix, fy = y-iy;
  
  return cubic(
    cubic(hash(ix-1, iy-1),
          hash(ix-1, iy),
          hash(ix-1, iy+1),
          hash(ix-1, iy+2), fy),
    cubic(hash(ix, iy-1),
          hash(ix, iy),
          hash(ix, iy+1),
          hash(ix, iy+2), fy),
    cubic(hash(ix+1, iy-1),
          hash(ix+1, iy),
          hash(ix+1, iy+1),
          hash(ix+1, iy+2), fy),
    cubic(hash(ix+2, iy-1),
          hash(ix+2, iy),
          hash(ix+2, iy+1),
          hash(ix+2, iy+2), fy),
    fx);
}

// this routine is pretty damn inefficient
float perlin_noise_2D(float x, float y, int oct)
{
  float acc = 0.0, strength = 1.0;
  int i;

  for (i = 0; i < oct; i++)
    {
      acc += strength * cubicxy (x, y);
      strength *= 0.5;
      x *= 2;
      y *= 2;
    }

  return acc;
}
