#ifndef GEOSPHERE_H
#define GEOSPHERE_H 1

#include "restrip.h"

extern float *make_geosphere (int depth, int *strips, int *strip_length);
extern strip *strips_for_geosphere (float *data, int num_strips,
				    int strip_length);

#endif
