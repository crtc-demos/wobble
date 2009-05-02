#ifndef RESTRIP_H
#define RESTRIP_H 1

typedef struct strip
{
  float (*start)[][3];
  /* Negative length for inverted strips?  */
  int length;
  void *extra;
  size_t extra_elsize;
  struct strip *next;
} strip;

typedef int (*strip_classify_fn) (float triangle[3][3], int clockwise,
				  void *extra);

extern void restrip_list (strip *strips_in, strip_classify_fn fn,
			  strip *strip_starts[], strip *strip_ends[],
			  strip **stripbuf, unsigned int *capacity);

#endif
