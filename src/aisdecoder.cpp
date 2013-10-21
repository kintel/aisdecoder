#include "aisdecoder.h"

extern "C" {
  #include "aivdm_decode.h" // gps_device_t
}

#include <string.h>

ais_handle_t *ais_create_handle()
{
  ais_handle_t *handle = new ais_handle_t;
  memset(&handle->driver.aivdm, 0, sizeof(handle->driver.aivdm));
  handle->context = new ais_handle_t::gps_context_t;
  
  return handle;
}

void ais_destroy_handle(ais_handle_t *handle)
{
  delete handle;
}

int ais_decode(ais_handle_t *handle,
               const char *buf, size_t buflen,
               struct ais_t *ais,
               bool split24,
               int debug)
{
  handle->context->debug = debug;
  return aivdm_decode(buf, buflen, handle, ais, split24, debug);
}
