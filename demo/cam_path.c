#include "object.h"
#include "cam_path.h"
#include "viewpoint.h"

static float
cubic (float v0, float v1, float v2, float v3, float x)
{
  float a = -(v0 - 3 * v1 + 3 * v2 - v3) / 6;
  float b = (v0 - 2 * v1 + v2) / 2;
  float c = -(2 * v0 + 3 * v1 - 6 * v2 + v3) / 6;
  float d = v1;

  return a * x * x * x + b * x * x + c * x + d;
}

static void
move_camera (uint32_t time_offset, void *params, int iparam, viewpoint *view,
	     lighting *lights)
{
  camera_path *path = params;
  uint32_t adjusted_time = time_offset + iparam;
  float fparam = (float) (adjusted_time % path->step_time)
		 / (float) path->step_time;
  int section = adjusted_time / path->step_time;
  int eye_sec = section * 2;
  int lookat_sec = section * 2 + 1;
  int i;
  float eye[3], lookat[3];
  
  if (section * 2 >= path->length - 8)
    return;
  
  for (i = 0; i < 3; i++)
    {
      eye[i] = cubic ((*path->path)[eye_sec][i], (*path->path)[eye_sec + 2][i],
		      (*path->path)[eye_sec + 4][i],
		      (*path->path)[eye_sec + 6][i], fparam);
      lookat[i] = cubic ((*path->path)[lookat_sec][i],
			 (*path->path)[lookat_sec + 2][i],
			 (*path->path)[lookat_sec + 4][i],
			 (*path->path)[lookat_sec + 6][i], fparam);
    }

  view_set_eye_pos (view, eye[0], eye[1], eye[2]);
  view_set_look_at (view, lookat[0], lookat[1], lookat[2]);
}

effect_methods cam_path_methods = {
  .preinit_assets = NULL,
  .init_effect = NULL,
  .prepare_frame = &move_camera,
  .display_effect = NULL,
  .uninit_effect = NULL
};
