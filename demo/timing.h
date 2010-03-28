#ifndef TIMING_H
#define TIMING_H 1

#include "object.h"

#include <stdint.h>

typedef struct {
  void (*preinit_assets) (void);
  void (*init_effect) (void *params);
  void (*prepare_frame) (uint32_t time_offset, void *params, int iparam,
			 viewpoint *view, lighting *lights);
  void (*display_effect) (uint32_t time_offset, void *params, int iparam,
			  viewpoint *view, lighting *lights, int pass);
  void (*uninit_effect) (void *params);
  void (*finalize) (void *params);
} effect_methods;

typedef struct {
  uint64_t start_time;
  uint64_t end_time;
  effect_methods *methods;
  void *params;
  int iparam;
  int finalize;
} do_thing_at;

extern float audio_amplitude (void);

#endif
