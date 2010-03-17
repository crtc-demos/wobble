#ifndef VIEWPOINT_H
#define VIEWPOINT_H 1

typedef struct viewpoint
{
  matrix_t *projection;
  matrix_t *camera;
  matrix_t *camera_orientation;
  matrix_t *inv_camera_orientation;
  float eye_pos[3];
  float look_at[3];
  float up_dir[3];
  float near;
  int dirty;
} viewpoint;

struct lighting;

extern void view_set_eye_pos (viewpoint *, float, float, float);
extern void view_set_look_at (viewpoint *, float, float, float);
extern void view_set_up_dir (viewpoint *, float, float, float);
extern void view_set_perspective (viewpoint *, float fov, float aspect,
				  float z_near, float z_far);
extern void view_fix (viewpoint *, struct lighting *);
extern viewpoint *view_allocate (void);
extern void view_free (viewpoint *);

#endif
