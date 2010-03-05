#ifndef DRAW_TORUS_H
#define DRAW_TORUS_H 1

#include <stdint.h>

#include "timing.h"

typedef struct {
  object_orientation obj_orient;
  uint8_t ambient_r, ambient_g, ambient_b;
  uint8_t diffuse_r, diffuse_g, diffuse_b;
} torus_params;

extern effect_methods torus_methods;

#endif
