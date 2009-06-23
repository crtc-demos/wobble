#ifndef ENVMAP_DUAL_PARA_H
#define ENVMAP_DUAL_PARA_H 1

#include <kos.h>

typedef struct
{
  pvr_ptr_t front_txr;
  pvr_ptr_t back_txr;
  unsigned int xsize;
  unsigned int ysize;
} envmap_dual_para_info;

#endif
