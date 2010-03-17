#include <GL/glu.h>

#include "object.h"
#include "viewpoint.h"

void
view_set_eye_pos (viewpoint *view, float x, float y, float z)
{
  view->eye_pos[0] = x;
  view->eye_pos[1] = y;
  view->eye_pos[2] = z;
  view->dirty = 1;
}

void
view_set_look_at (viewpoint *view, float x, float y, float z)
{
  view->look_at[0] = x;
  view->look_at[1] = y;
  view->look_at[2] = z;
  view->dirty = 1;
}

void
view_set_up_dir (viewpoint *view, float x, float y, float z)
{
  view->up_dir[0] = x;
  view->up_dir[1] = y;
  view->up_dir[2] = z;
  view->dirty = 1;
}

static matrix_t screen_mat __attribute__((aligned(32))) =
  {
    { 320.0,  0,     0, 0 },
    { 0,     -240.0, 0, 0 },
    { 0,      0,     1, 0 },
    { 320.0,  240.0, 0, 1 }
  };

void
view_set_perspective (viewpoint *view, float fov, float aspect, float z_near,
		      float z_far)
{
  glKosMatrixDirty ();
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (fov, aspect, z_near, z_far);
  glGetFloatv (GL_PROJECTION_MATRIX, &(*view->projection)[0][0]);
  view->near = -0.2f;
  
  mat_load (&screen_mat);
  mat_apply (view->projection);
  mat_store (view->projection);
  
  glKosMatrixDirty ();
  
  view->dirty = 1;
}

void
view_fix (viewpoint *view, lighting *lights)
{
  if (view->dirty)
    {
      unsigned int i;
      
      glKosMatrixDirty ();
      glMatrixMode (GL_MODELVIEW);
      glLoadIdentity ();
      gluLookAt (view->eye_pos[0], view->eye_pos[1], view->eye_pos[2],
		 view->look_at[0], view->look_at[1], view->look_at[2],
		 view->up_dir[0],  view->up_dir[1],  view->up_dir[2]);
      glGetFloatv (GL_MODELVIEW_MATRIX, &(*view->camera)[0][0]);
      vec_extract_rotation (&(*view->camera_orientation)[0][0],
			    &(*view->camera)[0][0]);
      vec_transpose_rotation (&(*view->inv_camera_orientation)[0][0],
			      &(*view->camera)[0][0]);
      
      view->dirty = 0;
      
      for (i = 0; i < MAX_LIGHTS; i++)
        lights->light[i].dirty = 1;
    }
}

viewpoint *
view_allocate (void)
{
  viewpoint *v = malloc (sizeof (viewpoint));

  v->projection = memalign (32, sizeof (matrix_t));
  v->camera = memalign (32, sizeof (matrix_t));
  v->camera_orientation = memalign (32, sizeof (matrix_t));
  v->inv_camera_orientation = memalign (32, sizeof (matrix_t));

  return v;
}

void
view_free (viewpoint *v)
{
  free (v->inv_camera_orientation);
  free (v->camera_orientation);
  free (v->camera);
  free (v->projection);
  free (v);
}
