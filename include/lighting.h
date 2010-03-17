#ifndef LIGHTING_H
#define LIGHTING_H 1

#include "colour.h"
#include "viewpoint.h"

typedef struct
{
  float pos[3];
  float up[3];
  colour light_colour;
  float pos_xform[3];
  float up_xform[3];
  int dirty;
} light_info;

#define MAX_LIGHTS 2

typedef struct lighting
{
  light_info light[MAX_LIGHTS];
  int active;
} lighting;

extern void light_set_pos (lighting *, int num, float x, float y, float z);
extern void light_set_up (lighting *, int num, float x, float y, float z);
extern void light_set_colour (lighting *, int num, int r, int g, int b);
extern void light_set_active (lighting *, int num);
extern void light_fix (viewpoint *, lighting *);

#endif
