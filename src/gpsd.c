#include "gpsd.h"
#include <stdarg.h>
#include <stdio.h>

void gpsd_report(const int debuglevel, const int errlevel,
                 const char *fmt, ...) {

  if (errlevel <= debuglevel) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
  }
}
