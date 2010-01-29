#ifndef CEL_SHADING_H
#define CEL_SHADING_H 1

typedef struct
{
  pvr_ptr_t texture;  /* Temporary!  */
  uint32 w, h;
  int info;  /* What do we want?  When do we want it?  */
} celshading_info;

#endif
