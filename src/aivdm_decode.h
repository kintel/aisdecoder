#ifndef GPSD_AIVDM_H_
#define GPSD_AIVDM_H_

#include "ais.h"

/*
 * For NMEA-conforming receivers this is supposed to be 82, but
 * some receivers (TN-200, GSW 2.3.2) emit oversized sentences.
 * The current hog champion is the Trimble BX-960 receiver, which
 * emits a 91-character GGA message.
 */
#define NMEA_MAX	91		/* max length of NMEA sentence */
#define NMEA_BIG_BUF	(2*NMEA_MAX+1)	/* longer than longest NMEA sentence */
/* state for resolving interleaved Type 24 packets */
struct ais_type24a_t {
    unsigned int mmsi;
    char shipname[AIS_SHIPNAME_MAXLEN+1];
};

#define MAX_TYPE24_INTERLEAVE	8	/* max number of queued type 24s */
struct ais_type24_queue_t {
    struct ais_type24a_t ships[MAX_TYPE24_INTERLEAVE];
    int index;
};

/* state for resolving AIVDM decodes */
struct aivdm_context_t {
    /* hold context for decoding AIDVM packet sequences */
    int decoded_frags;		/* for tracking AIDVM parts in a multipart sequence */
    unsigned char bits[2048];
    size_t bitlen; /* how many valid bits */
    struct ais_type24_queue_t type24_queue;
};

struct gps_device_t {
  struct {
#define AIVDM_CHANNELS	2		/* A, B */
    struct {
      struct aivdm_context_t context[AIVDM_CHANNELS];
      char ais_channel;
    } aivdm;
  } driver;
  struct gps_context_t {
    int debug;                    /* global debug level */
  } *context;
};

bool aivdm_decode(const char *buf, size_t buflen,
                  struct gps_device_t *session,
                  struct ais_t *ais,
                  bool split24, int debug);

#endif
