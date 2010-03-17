#include "object.h"
#include "lighting.h"

void
light_set_pos (lighting *lights, int num, float x, float y, float z)
{
  lights->light[num].pos[0] = x;
  lights->light[num].pos[1] = y;
  lights->light[num].pos[2] = z;
  lights->light[num].dirty = 1;
}

void
light_set_up (lighting *lights, int num, float x, float y, float z)
{
  lights->light[num].up[0] = x;
  lights->light[num].up[1] = y;
  lights->light[num].up[2] = z;
  lights->light[num].dirty = 1;
}

void
light_set_colour (lighting *lights, int num, int r, int g, int b)
{
  lights->light[num].light_colour.r = r;
  lights->light[num].light_colour.g = g;
  lights->light[num].light_colour.b = b;
  /* Doesn't affect transformed matrices; not dirty.  */
}

void
light_set_active (lighting *lights, int num)
{
  lights->active = num;
}

void
light_fix (viewpoint *view, lighting *lights)
{
  unsigned int i;

  if (view->dirty)
    view_fix (view, lights);
  
  mat_load (view->camera);
  
  for (i = 0; i < MAX_LIGHTS; i++)
    {
      if (lights->light[i].dirty)
        {
	  float x, y, z, w = 1.0;
	  
	  x = lights->light[i].pos[0];
	  y = lights->light[i].pos[1];
	  z = lights->light[i].pos[2];
	  mat_trans_nodiv (x, y, z, w);
	  lights->light[i].pos_xform[0] = x;
	  lights->light[i].pos_xform[1] = y;
	  lights->light[i].pos_xform[2] = z;
	}
    }

  mat_load (view->camera_orientation);

  for (i = 0; i < MAX_LIGHTS; i++)
    {
      if (lights->light[i].dirty)
        {
	  float x, y, z, w = 1.0;
	  
	  x = lights->light[i].up[0];
	  y = lights->light[i].up[1];
	  z = lights->light[i].up[2];
	  mat_trans_nodiv (x, y, z, w);
	  lights->light[i].up_xform[0] = x;
	  lights->light[i].up_xform[1] = y;
	  lights->light[i].up_xform[2] = z;

	  lights->light[i].dirty = 0;
	}
    }
  
  glKosMatrixDirty ();
}
