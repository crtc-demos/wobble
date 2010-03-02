#ifndef TORUS_H
#define TORUS_H 1

typedef struct {
  uint32_t colour;
} torus_params;

extern void draw_torus (uint64_t time_offset, void *params);

#endif
