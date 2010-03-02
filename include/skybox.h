#ifndef SKYBOX_H
#define SKYBOX_H 1

#include "object.h"

extern object *create_skybox (float radius, pvr_ptr_t textures[6],
			      unsigned int xsize, unsigned int ysize);

#endif
