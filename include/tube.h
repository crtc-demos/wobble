#ifndef TUBE_H
#define TUBE_H 1

#include "object.h"

extern object *allocate_tube (int rows, int segments);
extern void fill_tube_data (object *obj, int rows, int segments, float rot1);

#endif
