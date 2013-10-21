#include "aivdm_decode.h"
#include "gpsd.h" // for gpsd_report()
#include "driver_ais.h" // for ais_binary_decode()

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/**************************************************************************
 *
 * AIVDM - ASCII armoring of binary AIS packets
 *
 **************************************************************************/

/*@ -fixedformalarray -usedef -branchstate @*/
bool aivdm_decode(const char *buf, size_t buflen,
		  struct gps_device_t *session,
		  struct ais_t *ais,
		  bool split24, int debug)
{
#ifdef __UNUSED_DEBUG__
    char *sixbits[64] = {
	"000000", "000001", "000010", "000011", "000100",
	"000101", "000110", "000111", "001000", "001001",
	"001010", "001011", "001100", "001101", "001110",
	"001111", "010000", "010001", "010010", "010011",
	"010100", "010101", "010110", "010111", "011000",
	"011001", "011010", "011011", "011100", "011101",
	"011110", "011111", "100000", "100001", "100010",
	"100011", "100100", "100101", "100110", "100111",
	"101000", "101001", "101010", "101011", "101100",
	"101101", "101110", "101111", "110000", "110001",
	"110010", "110011", "110100", "110101", "110110",
	"110111", "111000", "111001", "111010", "111011",
	"111100", "111101", "111110", "111111",
    };
#endif /* __UNUSED_DEBUG__ */
    int nfrags, ifrag, nfields = 0;
    unsigned char *field[NMEA_MAX*2];
    unsigned char fieldcopy[NMEA_MAX*2+1];
    unsigned char *data, *cp;
    unsigned char ch, pad;
    struct aivdm_context_t *ais_context;
    int i;

    if (buflen == 0)
	return false;

    /* we may need to dump the raw packet */
    gpsd_report(session->context->debug, LOG_PROG,
		"AIVDM packet length %zd: %s\n", buflen, buf);

    /* first clear the result, making sure we don't return garbage */
    memset(ais, 0, sizeof(*ais));

    /* discard overlong sentences */
    if (strlen(buf) > sizeof(fieldcopy)-1) {
	gpsd_report(session->context->debug, LOG_ERROR, "overlong AIVDM packet.\n");
	return false;
    }

    /* extract packet fields */
    (void)strlcpy((char *)fieldcopy, buf, sizeof(fieldcopy));
    field[nfields++] = (unsigned char *)buf;
    for (cp = fieldcopy;
	 cp < fieldcopy + buflen; cp++)
	if (*cp == (unsigned char)',') {
	    *cp = '\0';
	    field[nfields++] = cp + 1;
	}

    /* discard sentences with exiguous commas; catches run-ons */
    if (nfields < 7) {
	gpsd_report(session->context->debug, LOG_ERROR, "malformed AIVDM packet.\n");
	return false;
    }

    switch (field[4][0]) {
    /* FIXME: if fields[4] == "12", it doesn't detect the error */
    case '\0':
	/*
	 * Apparently an empty channel is normal for AIVDO sentences,
	 * which makes sense as they don't come in over radio.  This
	 * is going to break if there's ever an AIVDO type 24, though.
	 */
	if (strncmp((const char *)field[0], "!AIVDO", 6) != 0)
	    gpsd_report(session->context->debug, LOG_INF,
			"invalid empty AIS channel. Assuming 'A': [%s]", field[0]);
	ais_context = &session->driver.aivdm.context[0];
	session->driver.aivdm.ais_channel ='A';
	break;
    case '1':
	/*@fallthrough@*/
    case 'A':
	ais_context = &session->driver.aivdm.context[0];
	session->driver.aivdm.ais_channel ='A';
	break;
    case '2':
	/*@fallthrough@*/
    case 'B':
	ais_context = &session->driver.aivdm.context[1];
	session->driver.aivdm.ais_channel ='B';
	break;
    case 'C':
	gpsd_report(session->context->debug, LOG_INF,
		    "ignoring AIS channel C (secure AIS).\n");
        return false;
        break;
    default:
	gpsd_report(session->context->debug, LOG_ERROR,
		    "invalid AIS channel 0x%0X .\n", field[4][0]);
	return false;
    }

    nfrags = atoi((char *)field[1]); /* number of fragments to expect */
    ifrag = atoi((char *)field[2]); /* fragment id */
    data = field[5];
    pad = field[6][0]; /* number of padding bits */
    gpsd_report(session->context->debug, LOG_PROG,
		"nfrags=%d, ifrag=%d, decoded_frags=%d, data=%s\n",
		nfrags, ifrag, ais_context->decoded_frags, data);

    /* assemble the binary data */

    /* check fragment ordering */
    if (ifrag != ais_context->decoded_frags + 1) {
	gpsd_report(session->context->debug, LOG_ERROR,
		    "invalid fragment #%d received, expected #%d.\n",
		    ifrag, ais_context->decoded_frags + 1);
	if (ifrag != 1)
	    return false;
        /* else, ifrag==1: Just discard all that was previously decoded and
         * simply handle that packet */
        ais_context->decoded_frags = 0;
    }
    if (ifrag == 1) {
	(void)memset(ais_context->bits, '\0', sizeof(ais_context->bits));
	ais_context->bitlen = 0;
    }

    /* wacky 6-bit encoding, shades of FIELDATA */
    /*@ +charint @*/
    for (cp = data; cp < data + strlen((char *)data); cp++) {
	ch = *cp;
	ch -= 48;
	if (ch >= 40)
	    ch -= 8;
#ifdef __UNUSED_DEBUG__
	gpsd_report(session->context->debug, LOG_RAW,
		    "%c: %s\n", *cp, sixbits[ch]);
#endif /* __UNUSED_DEBUG__ */
	/*@ -shiftnegative @*/
	for (i = 5; i >= 0; i--) {
	    if ((ch >> i) & 0x01) {
		ais_context->bits[ais_context->bitlen / 8] |=
		    (1 << (7 - ais_context->bitlen % 8));
	    }
	    ais_context->bitlen++;
	    if (ais_context->bitlen > sizeof(ais_context->bits)) {
		gpsd_report(session->context->debug, LOG_INF,
			    "overlong AIVDM payload truncated.\n");
		return false;
	    }
	}
	/*@ +shiftnegative @*/
    }
    if (isdigit(pad))
	ais_context->bitlen -= (pad - '0');	/* ASCII assumption */
    /*@ -charint @*/

    /* time to pass buffered-up data to where it's actually processed? */
    if (ifrag == nfrags) {
#if 0
	if (debug >= LOG_INF) {
	    size_t clen = (ais_context->bitlen + 7) / 8;
	    gpsd_report(session->context->debug, LOG_INF,
			"AIVDM payload is %zd bits, %zd chars: %s\n",
			ais_context->bitlen, clen,
			gpsd_hexdump(session->msgbuf, sizeof(session->msgbuf),
				     (char *)ais_context->bits, clen));
	}
#endif
        /* clear waiting fragments count */
        ais_context->decoded_frags = 0;

	/* decode the assembled binary packet */
	return ais_binary_decode(session->context->debug,
				 ais,
				 ais_context->bits,
				 ais_context->bitlen,
				 split24 ? NULL : &ais_context->type24_queue);
    }

    /* we're still waiting on another sentence */
    ais_context->decoded_frags++;
    return false;
}
/*@ +fixedformalarray +usedef +branchstate @*/
