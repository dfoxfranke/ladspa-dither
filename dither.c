#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ladspa.h"

#define RAND_STRIDE 16

#define DITHER_PORT_INPUT 0
#define DITHER_PORT_OUTPUT 1
#define DITHER_PORT_PRECISION 2

#define DITHER_UNIQUEID 5341

struct handle {
  unsigned rand_state_a[RAND_STRIDE];
  unsigned rand_state_b[RAND_STRIDE];  
  LADSPA_Data* port_data[3];
  float gain;
};

static inline void update_rand_state(struct handle * restrict handle, float * restrict a, float * restrict b) {
  int i;
  for(i=0;i<RAND_STRIDE;i++) {
    handle->rand_state_a[i] = (0x43fd43fd * handle->rand_state_a[i] + 0xc39ec6) & 0xffffff;
    handle->rand_state_b[i] = (0x43fd43fd * handle->rand_state_b[i] + 0xc39ec6) & 0xffffff;    
    a[i] = (float)handle->rand_state_a[i] * 0x1p-24f;
    b[i] = (float)handle->rand_state_b[i] * 0x1p-24f;    
  }
}

static LADSPA_Handle dither_instantiate(LADSPA_Descriptor const* descriptor,
                                        unsigned long sample_rate) {
  return calloc(1, sizeof (struct handle));
}

static void dither_connect_port(LADSPA_Handle instance,
                                unsigned long port,
                                LADSPA_Data *data_location) {
  struct handle *handle = (struct handle*)instance;
  handle->port_data[port] = data_location;
}

static void dither_activate(LADSPA_Handle instance) {
  struct handle* handle = (struct handle*)instance;
  unsigned i;
  for(i=0; i < RAND_STRIDE; i++) {
    handle->rand_state_a[i] = i;
    handle->rand_state_b[i] = i + RAND_STRIDE;
  }
}

static void dither_run(LADSPA_Handle instance,
                       unsigned long sample_count) {
  unsigned long i,j;
  struct handle *handle = (struct handle*)instance;
  float random_a[RAND_STRIDE];
  float random_b[RAND_STRIDE];

  float precision = exp2f(1.0f - *handle->port_data[DITHER_PORT_PRECISION]);
  
  for(i = 0; i + RAND_STRIDE <= sample_count; i+= RAND_STRIDE) {
    update_rand_state(handle, random_a, random_b);
    for(j=0; j < RAND_STRIDE; j++) {
      handle->port_data[DITHER_PORT_OUTPUT][i+j] =
        handle->port_data[DITHER_PORT_INPUT][i+j] +
        precision *
        (random_a[j] + random_b[j] - 1.0f);
    }
  }

  update_rand_state(handle, random_a, random_b);
  for(j=0; i+j < sample_count; j++) {
    handle->port_data[DITHER_PORT_OUTPUT][i+j] =
      handle->port_data[DITHER_PORT_INPUT][i+j] +
      precision *
      (random_a[j] + random_b[j] - 1.0f);
  }
}

static void dither_run_adding(LADSPA_Handle instance,
                              unsigned long sample_count) {
  unsigned long i,j;
  struct handle *handle = (struct handle*)instance;
  float random_a[RAND_STRIDE];
  float random_b[RAND_STRIDE];

  float precision = exp2f(1.0f - *handle->port_data[DITHER_PORT_PRECISION]);
  
  for(i = 0; i + RAND_STRIDE <= sample_count; i+= RAND_STRIDE) {
    update_rand_state(handle, random_a, random_b);
    for(j=0; j < RAND_STRIDE; j++) {
      handle->port_data[DITHER_PORT_OUTPUT][i+j] +=
        handle->gain *
        (handle->port_data[DITHER_PORT_INPUT][i+j] +
         precision *
         (random_a[j] + random_b[j] - 1.0f));
    }
  }

  update_rand_state(handle, random_a, random_b);
  for(j=0; i+j < sample_count; j++) {
    handle->port_data[DITHER_PORT_OUTPUT][i+j] +=
      handle->gain *
      (handle->port_data[DITHER_PORT_INPUT][i+j] +
       precision *
       (random_a[j] + random_b[j] - 1.0f));
  }
}


static void dither_set_run_adding_gain(LADSPA_Handle instance,
                                       LADSPA_Data gain) {
  ((struct handle*)instance)->gain = gain;
}

static void dither_cleanup(LADSPA_Handle instance) {
  free(instance);
}

static const LADSPA_Descriptor dither_descriptor = {
  .UniqueID = DITHER_UNIQUEID,
  .Label = "dither",
  .Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE,
  .Name = "Dither",
  .Maker = "Daniel Fox Franke <dfoxfranke@gmail.com>",
  .Copyright = "Copyright (c) 2017 Daniel Fox Franke",
  .PortCount = 3,
  .PortDescriptors = (const LADSPA_PortDescriptor[]){
    LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL
  },
  .PortNames = (char const* const[]){
    "input", "output", "precision"
  },
  .PortRangeHints = (const LADSPA_PortRangeHint[]){
    {LADSPA_HINT_DEFAULT_NONE, 0.0f, 0.0f},
    {LADSPA_HINT_DEFAULT_NONE, 0.0f, 0.0f},
    {LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER |
     LADSPA_HINT_DEFAULT_MAXIMUM, 1.0f, 24.0f}
  },
  .ImplementationData = NULL,
  .instantiate = dither_instantiate,
  .connect_port = dither_connect_port,
  .activate = dither_activate,
  .run = dither_run,
  .run_adding = dither_run_adding,
  .set_run_adding_gain = dither_set_run_adding_gain,
  .deactivate = NULL,
  .cleanup = dither_cleanup
};

LADSPA_Descriptor const* ladspa_descriptor(unsigned long index) {
  if(index == 0) return &dither_descriptor;
  return NULL;
}
