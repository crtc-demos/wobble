#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H 1

#include "object.h"

typedef struct
{
  /* A light source.  */
  colour ambient_light;
  vector light_pos;
  vector light_updir;
  
  /* Camera matrix.  */
  matrix_t *camera;
  /* Inverse of camera matrix, rotation part only.  */
  matrix_t *inv_camera_rot;
  
  /* Projection matrix.  */
  matrix_t *projection;
} environment;

#endif
