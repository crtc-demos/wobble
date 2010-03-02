#ifndef TORUS_H
#define TORUS_H 1

#include "object.h"

extern object *torus_create (float outer, float inner, int minor_steps,
			     int major_steps);

#endif
