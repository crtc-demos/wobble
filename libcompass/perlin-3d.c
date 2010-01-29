#include "perlin.h"

static float hash(int x, int y, int z)
{
  x += y*1471;
  x += z*15491;
  x = (x<<13) ^ x;
  return (1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff)
          / 1073741824.0);
}

/* 2D smoothed noise weightings:

  1  2  1
  2  4  2
  1  2  1

*/

static float smooth(int x, int y)
{
/*  return ((hash(x-1, y-1) + hash(x+1, y-1) + hash(x-1, y+1) + hash(x+1, y+1)) +
          (hash(x-1, y) + hash(x+1, y) + hash(x, y-1) + hash(x, y+1))*2.0 +
           hash(x,y)*4.0)/16.0;*/
  return hash(x,y, 0);
}

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

static float cubicxy(float x, float y, float z)
{
  int ix = (int)x, iy = (int)y, iz = (int)z;
  float fx = x-ix, fy = y-iy, fz = z-iz;
  int i, j, k;
  float sm[4][4][4];

  for (i=0; i<4; i++)
    for (j=0; j<4; j++)
      for (k=0; k<4; k++)
        sm[i][j][k] = hash(i-1, j-1, k-1);

  for (i=0; i<4; i++)
    for (j=0; j<4; j++)
      sm[i][j][0] = cubic(sm[i][j][0],
                          sm[i][j][1],
                          sm[i][j][2],
                          sm[i][j][3], fz);
  for (i=0; i<4; i++)
    sm[i][0][0] = cubic(sm[i][0][0],
                        sm[i][1][0],
                        sm[i][2][0],
                        sm[i][3][0], fy);


  return cubic(sm[0][0][0],
	       sm[1][0][0],
	       sm[2][0][0],
	       sm[3][0][0],
               fx);
}

// this routine is pretty damn inefficient
float perlin_noise_3D(float x, float y, float z, int oct)
{
  return oct==0 ? 0.0 : cubicxy(x, y, z) +
    0.5*perlin_noise_3D(x*2.0, y*2.0, z*2.0, oct-1);
}
