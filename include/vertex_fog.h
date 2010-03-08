#ifndef VERTEX_FOG_H
#define VERTEX_FOG_H 1

struct vertex_attrs;

typedef struct
{
  pvr_ptr_t texture;  /* Temporary!  */
  uint32 w, h;
  float (*fogging) (float x, float y, float z, struct vertex_attrs *v_attr);
} vertexfog_info;

#endif
