#ifndef AISDECODER_H_
#define AISDECODER_H_

/*!
  This is a C wrapper for AIS decoding functionality extracted from the gpsd project
  (http://www.catb.org/gpsd).
*/

#include <stddef.h>
extern "C" {
#include "ais.h"
#include "gpsd.h"
}

typedef struct gps_device_t ais_handle_t;

ais_handle_t *ais_create_handle();
void ais_destroy_handle(ais_handle_t *handle);
int ais_decode(ais_handle_t *handle,
               const char *buf, size_t buflen,
               struct ais_t *ais,
               bool split24, 
               int debug);

#endif
