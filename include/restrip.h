#ifndef RESTRIP_H
#define RESTRIP_H 1

#include "object.h"

typedef void* (*restrip_allocator_fn) (unsigned int amt);

typedef int (*strip_classify_fn) (float triangle[3][3], int clockwise,
				  vertex_attrs *attrs);

extern void restrip_list (strip *strips_in, strip_classify_fn fn,
			  strip *strip_starts[], strip *strip_ends[],
			  restrip_allocator_fn alloc);

#endif
