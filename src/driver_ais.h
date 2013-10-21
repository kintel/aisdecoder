#ifndef DRIVER_AIS_H_
#define DRIVER_AIS_H_

bool ais_binary_decode(const int debug,
                       struct ais_t *ais,
                       const unsigned char *, size_t,
                       /*@null@*/struct ais_type24_queue_t *);

#endif
