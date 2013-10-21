#ifndef GPSD_H_

#include <sys/types.h> // needed by driver_ais.c
#include "ais.h" // needed by driver_ais.c

/* logging levels */
#define LOG_ERROR 	-1	/* errors, display always */
#define LOG_SHOUT	0	/* not an error but we should always see it */
#define LOG_WARN	1	/* not errors but may indicate a problem */
#define LOG_INF 	2	/* key informative messages */
#define LOG_DATA	3	/* log data management messages */
#define LOG_PROG	4	/* progress messages */
#define LOG_IO  	5	/* IO to and from devices */
#define LOG_SPIN	6	/* logging for catching spin bugs */
#define LOG_RAW 	7	/* raw low-level I/O */

void gpsd_report(const int debuglevel, const int errlevel, const char *fmt, ...);

#define MAX_PACKET_LENGTH	516	/* 7 + 506 + 3 */
extern /*@ observer @*/ const char *gpsd_hexdump(/*@out@*/char *, size_t,
						 /*@null@*/char *, size_t);

/* Some libcs do not have strlcat/strlcpy. Local copies are provided */
#ifndef HAVE_STRLCAT
# ifdef __cplusplus
extern "C" {
# endif
size_t strlcat(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif
#ifndef HAVE_STRLCPY
# ifdef __cplusplus
extern "C" {
# endif
size_t strlcpy(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif

#endif
