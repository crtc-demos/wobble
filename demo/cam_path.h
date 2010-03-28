#ifndef CAM_PATH_H
#define CAM_PATH_H 1

#include "timing.h"

typedef struct {
  float (*path)[][3];
  unsigned int length;
  unsigned int step_time;
} camera_path;

extern effect_methods cam_path_methods;

#endif
