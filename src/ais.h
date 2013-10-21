#ifndef AIS_H_
#define AIS_H_

/*!
  This header contains the ais_t struct, plus some support structs.
  The content is extracted from gps.h from the gpsd project.
*/

#include <stdbool.h>
#include <stddef.h>

#define AIVDM_ENABLE 1

/*
 * Is an MMSI number that of an auxiliary associated with a mother ship?
 * We need to be able to test this for decoding AIS Type 24 messages.
 * According to <http://www.navcen.uscg.gov/marcomms/gmdss/mmsi.htm#format>,
 * auxiliary-craft MMSIs have the form 98MIDXXXX, where MID is a country
 * code and XXXX the vessel ID.
 */
#define AIS_AUXILIARY_MMSI(n)	((n) / 10000000 == 98)

/* N/A values and scaling constant for 25/24 bit lon/lat pairs */
#define AIS_LON3_NOT_AVAILABLE	181000
#define AIS_LAT3_NOT_AVAILABLE	91000
#define AIS_LATLON3_DIV	60000.0

/* N/A values and scaling constant for 28/27 bit lon/lat pairs */
#define AIS_LON4_NOT_AVAILABLE	1810000
#define AIS_LAT4_NOT_AVAILABLE	910000
#define AIS_LATLON4_DIV	600000.0

struct route_info {
    unsigned int linkage;	/* Message Linkage ID */
    unsigned int sender;	/* Sender Class */
    unsigned int rtype;		/* Route Type */
    unsigned int month;		/* Start month */
    unsigned int day;		/* Start day */
    unsigned int hour;		/* Start hour */
    unsigned int minute;	/* Start minute */
    unsigned int duration;	/* Duration */
    int waycount;		/* Waypoint count */
    struct waypoint_t {
	signed int lon;		/* Longitude */
	signed int lat;		/* Latitude */
    } waypoints[16];
};

struct ais_t
{
    unsigned int	type;		/* message type */
    unsigned int    	repeat;		/* Repeat indicator */
    unsigned int	mmsi;		/* MMSI */
    union {
	/* Types 1-3 Common navigation info */
	struct {
	    unsigned int status;		/* navigation status */
	    signed turn;			/* rate of turn */
#define AIS_TURN_HARD_LEFT	-127
#define AIS_TURN_HARD_RIGHT	127
#define AIS_TURN_NOT_AVAILABLE	128
	    unsigned int speed;			/* speed over ground in deciknots */
#define AIS_SPEED_NOT_AVAILABLE	1023
#define AIS_SPEED_FAST_MOVER	1022		/* >= 102.2 knots */
	    bool accuracy;			/* position accuracy */
#define AIS_LATLON_DIV	600000.0
	    int lon;				/* longitude */
#define AIS_LON_NOT_AVAILABLE	0x6791AC0
	    int lat;				/* latitude */
#define AIS_LAT_NOT_AVAILABLE	0x3412140
	    unsigned int course;		/* course over ground */
#define AIS_COURSE_NOT_AVAILABLE	3600
	    unsigned int heading;		/* true heading */
#define AIS_HEADING_NOT_AVAILABLE	511
	    unsigned int second;		/* seconds of UTC timestamp */
#define AIS_SEC_NOT_AVAILABLE	60
#define AIS_SEC_MANUAL		61
#define AIS_SEC_ESTIMATED	62
#define AIS_SEC_INOPERATIVE	63
	    unsigned int maneuver;	/* maneuver indicator */
	    //unsigned int spare;	spare bits */
	    bool raim;			/* RAIM flag */
	    unsigned int radio;		/* radio status bits */
	} type1;
	/* Type 4 - Base Station Report & Type 11 - UTC and Date Response */
	struct {
	    unsigned int year;			/* UTC year */
#define AIS_YEAR_NOT_AVAILABLE	0
	    unsigned int month;			/* UTC month */
#define AIS_MONTH_NOT_AVAILABLE	0
	    unsigned int day;			/* UTC day */
#define AIS_DAY_NOT_AVAILABLE	0
	    unsigned int hour;			/* UTC hour */
#define AIS_HOUR_NOT_AVAILABLE	24
	    unsigned int minute;		/* UTC minute */
#define AIS_MINUTE_NOT_AVAILABLE	60
	    unsigned int second;		/* UTC second */
#define AIS_SECOND_NOT_AVAILABLE	60
	    bool accuracy;		/* fix quality */
	    int lon;			/* longitude */
	    int lat;			/* latitude */
	    unsigned int epfd;		/* type of position fix device */
	    //unsigned int spare;	spare bits */
	    bool raim;			/* RAIM flag */
	    unsigned int radio;		/* radio status bits */
	} type4;
	/* Type 5 - Ship static and voyage related data */
	struct {
	    unsigned int ais_version;	/* AIS version level */
	    unsigned int imo;		/* IMO identification */
	    char callsign[7+1];		/* callsign */
#define AIS_SHIPNAME_MAXLEN	20
	    char shipname[AIS_SHIPNAME_MAXLEN+1];	/* vessel name */
	    unsigned int shiptype;	/* ship type code */
	    unsigned int to_bow;	/* dimension to bow */
	    unsigned int to_stern;	/* dimension to stern */
	    unsigned int to_port;	/* dimension to port */
	    unsigned int to_starboard;	/* dimension to starboard */
	    unsigned int epfd;		/* type of position fix deviuce */
	    unsigned int month;		/* UTC month */
	    unsigned int day;		/* UTC day */
	    unsigned int hour;		/* UTC hour */
	    unsigned int minute;	/* UTC minute */
	    unsigned int draught;	/* draft in meters */
	    char destination[20+1];	/* ship destination */
	    unsigned int dte;		/* data terminal enable */
	    //unsigned int spare;	spare bits */
	} type5;
	/* Type 6 - Addressed Binary Message */
	struct {
	    unsigned int seqno;		/* sequence number */
	    unsigned int dest_mmsi;	/* destination MMSI */
	    bool retransmit;		/* retransmit flag */
	    //unsigned int spare;	spare bit(s) */
	    unsigned int dac;           /* Application ID */
	    unsigned int fid;           /* Functional ID */
#define AIS_TYPE6_BINARY_MAX	920	/* 920 bits */
	    size_t bitcount;		/* bit count of the data */
	    union {
		char bitdata[(AIS_TYPE6_BINARY_MAX + 7) / 8];
		/* Inland AIS - ETA at lock/bridge/terminal */
		struct {
		    char country[2+1];	/* UN Country Code */
		    char locode[3+1];	/* UN/LOCODE */
		    char section[5+1];	/* Fairway section */
		    char terminal[5+1];	/* Terminal code */
		    char hectometre[5+1];	/* Fairway hectometre */
		    unsigned int month;	/* ETA month */
		    unsigned int day;	/* ETA day */
		    unsigned int hour;	/* ETA hour */
		    unsigned int minute;	/* ETA minute */
		    unsigned int tugs;	/* Assisting Tugs */
		    unsigned int airdraught;	/* Air Draught */
		} dac200fid21;
		/* Inland AIS - ETA at lock/bridge/terminal */
		struct {
		    char country[2+1];	/* UN Country Code */
		    char locode[3+1];	/* UN/LOCODE */
		    char section[5+1];	/* Fairway section */
		    char terminal[5+1];	/* Terminal code */
		    char hectometre[5+1];	/* Fairway hectometre */
		    unsigned int month;	/* RTA month */
		    unsigned int day;	/* RTA day */
		    unsigned int hour;	/* RTA hour */
		    unsigned int minute;	/* RTA minute */
		    unsigned int status;	/* Status */
#define DAC200FID22_STATUS_OPERATIONAL	0
#define DAC200FID22_STATUS_LIMITED	1
#define DAC200FID22_STATUS_OUT_OF_ORDER	2
#define DAC200FID22_STATUS_NOT_AVAILABLE	0
		} dac200fid22;
		/* Inland AIS - Number of persons on board */
		struct {
		    unsigned int crew;	/* # crew on board */
		    unsigned int passengers;	/* # passengers on board */
		    unsigned int personnel;	/* # personnel on board */
#define DAC200FID55_COUNT_NOT_AVAILABLE	255
		} dac200fid55;
		/* GLA - AtoN monitoring data (UK/ROI) */
		struct {
		    unsigned int ana_int;       /* Analogue (internal) */
		    unsigned int ana_ext1;      /* Analogue (external #1) */
		    unsigned int ana_ext2;      /* Analogue (external #2) */
		    unsigned int racon; /* RACON status */
		    unsigned int light; /* Light status */
		    bool alarm; /* Health alarm*/
		    unsigned int stat_ext;      /* Status bits (external) */
		    bool off_pos;    /* Off position status */
		} dac235fid10;
		/* IMO236 - Dangerous Cargo Indication */
		struct {
		    char lastport[5+1];		/* Last Port Of Call */
		    unsigned int lmonth;	/* ETA month */
		    unsigned int lday;		/* ETA day */
		    unsigned int lhour;		/* ETA hour */
		    unsigned int lminute;	/* ETA minute */
		    char nextport[5+1];		/* Next Port Of Call */
		    unsigned int nmonth;	/* ETA month */
		    unsigned int nday;		/* ETA day */
		    unsigned int nhour;		/* ETA hour */
		    unsigned int nminute;	/* ETA minute */
		    char dangerous[20+1];	/* Main Dangerous Good */
		    char imdcat[4+1];		/* IMD Category */
		    unsigned int unid;		/* UN Number */
		    unsigned int amount;	/* Amount of Cargo */
		    unsigned int unit;		/* Unit of Quantity */
		} dac1fid12;
		/* IMO236 - Extended Ship Static and Voyage Related Data */
		struct {
		    unsigned int airdraught;	/* Air Draught */
		} dac1fid15;
		/* IMO236 - Number of Persons on board */
		struct {
		    unsigned persons;	/* number of persons */
		} dac1fid16;
		/* IMO289 - Clearance Time To Enter Port */
		struct {
		    unsigned int linkage;	/* Message Linkage ID */
		    unsigned int month;	/* Month (UTC) */
		    unsigned int day;	/* Day (UTC) */
		    unsigned int hour;	/* Hour (UTC) */
		    unsigned int minute;	/* Minute (UTC) */
		    char portname[20+1];	/* Name of Port & Berth */
		    char destination[5+1];	/* Destination */
		    signed int lon;	/* Longitude */
		    signed int lat;	/* Latitude */
		} dac1fid18;
		/* IMO289 - Berthing Data (addressed) */
		struct {
		    unsigned int linkage;	/* Message Linkage ID */
		    unsigned int berth_length;	/* Berth length */
		    unsigned int berth_depth;	/* Berth Water Depth */
		    unsigned int position;	/* Mooring Position */
		    unsigned int month;	/* Month (UTC) */
		    unsigned int day;	/* Day (UTC) */
		    unsigned int hour;	/* Hour (UTC) */
		    unsigned int minute;	/* Minute (UTC) */
		    unsigned int availability;	/* Services Availability */
		    unsigned int agent;	/* Agent */
		    unsigned int fuel;	/* Bunker/fuel */
		    unsigned int chandler;	/* Chandler */
		    unsigned int stevedore;	/* Stevedore */
		    unsigned int electrical;	/* Electrical */
		    unsigned int water;	/* Potable water */
		    unsigned int customs;	/* Customs house */
		    unsigned int cartage;	/* Cartage */
		    unsigned int crane;	/* Crane(s) */
		    unsigned int lift;	/* Lift(s) */
		    unsigned int medical;	/* Medical facilities */
		    unsigned int navrepair;	/* Navigation repair */
		    unsigned int provisions;	/* Provisions */
		    unsigned int shiprepair;	/* Ship repair */
		    unsigned int surveyor;	/* Surveyor */
		    unsigned int steam;	/* Steam */
		    unsigned int tugs;	/* Tugs */
		    unsigned int solidwaste;	/* Waste disposal (solid) */
		    unsigned int liquidwaste;	/* Waste disposal (liquid) */
		    unsigned int hazardouswaste;	/* Waste disposal (hazardous) */
		    unsigned int ballast;	/* Reserved ballast exchange */
		    unsigned int additional;	/* Additional services */
		    unsigned int regional1;	/* Regional reserved 1 */
		    unsigned int regional2;	/* Regional reserved 2 */
		    unsigned int future1;	/* Reserved for future */
		    unsigned int future2;	/* Reserved for future */
		    char berth_name[20+1];	/* Name of Berth */
		    signed int berth_lon;	/* Longitude */
		    signed int berth_lat;	/* Latitude */
		} dac1fid20;
		/* IMO289 - Weather observation report from ship */
		/*** WORK IN PROGRESS - NOT YET DECODED ***/
		struct {
		    bool wmo;			/* true if WMO variant */
		    union {
			struct {
			    char location[20+1];	/* Location */
			    signed int lon;		/* Longitude */
			    signed int lat;		/* Latitude */
			    unsigned int day;		/* Report day */
			    unsigned int hour;		/* Report hour */
			    unsigned int minute;	/* Report minute */
			    bool vislimit;		/* Max range? */
			    unsigned int visibility;	/* Units of 0.1 nm */
#define DAC1FID21_VISIBILITY_NOT_AVAILABLE	127
#define DAC1FID21_VISIBILITY_SCALE		10.0
			    unsigned humidity;		/* units of 1% */
			    unsigned int wspeed;	/* average wind speed */
			    unsigned int wgust;		/* wind gust */
#define DAC1FID21_WSPEED_NOT_AVAILABLE		127
			    unsigned int wdir;		/* wind direction */
#define DAC1FID21_WDIR_NOT_AVAILABLE		360
			    unsigned int pressure;	/* air pressure, hpa */
#define DAC1FID21_NONWMO_PRESSURE_NOT_AVAILABLE	403
#define DAC1FID21_NONWMO_PRESSURE_HIGH		402	/* > 1200hPa */
#define DAC1FID21_NONWMO_PRESSURE_OFFSET		400	/* N/A */
			    unsigned int pressuretend;	/* tendency */
		    	    int airtemp;		/* temp, units 0.1C */
#define DAC1FID21_AIRTEMP_NOT_AVAILABLE		-1024
#define DAC1FID21_AIRTEMP_SCALE			10.0
			    unsigned int watertemp;	/* units 0.1degC */
#define DAC1FID21_WATERTEMP_NOT_AVAILABLE	501
#define DAC1FID21_WATERTEMP_SCALE		10.0
			    unsigned int waveperiod;	/* in seconds */
#define DAC1FID21_WAVEPERIOD_NOT_AVAILABLE	63
			    unsigned int wavedir;	/* direction in deg */
#define DAC1FID21_WAVEDIR_NOT_AVAILABLE		360
			    unsigned int swellheight;	/* in decimeters */
			    unsigned int swellperiod;	/* in seconds */
			    unsigned int swelldir;	/* direction in deg */
			} nonwmo_obs;
			struct {
			    signed int lon;		/* Longitude */
			    signed int lat;		/* Latitude */
			    unsigned int month;		/* UTC month */
			    unsigned int day;		/* Report day */
			    unsigned int hour;		/* Report hour */
			    unsigned int minute;	/* Report minute */
			    unsigned int course;	/* course over ground */
			    unsigned int speed;		/* speed, m/s */
#define DAC1FID21_SOG_NOT_AVAILABLE		31
#define DAC1FID21_SOG_HIGH_SPEED		30
#define DAC1FID21_SOG_SCALE			2.0
			    unsigned int heading;	/* true heading */
#define DAC1FID21_HDG_NOT_AVAILABLE		127
#define DAC1FID21_HDG_SCALE			5.0
			    unsigned int pressure;	/* units of hPa * 0.1 */
#define DAC1FID21_WMO_PRESSURE_SCALE		10
#define DAC1FID21_WMO_PRESSURE_OFFSET		90.0
			    unsigned int pdelta;	/* units of hPa * 0.1 */
#define DAC1FID21_PDELTA_SCALE			10
#define DAC1FID21_PDELTA_OFFSET			50.0
			    unsigned int ptend;		/* enumerated */
			    unsigned int twinddir;	/* in 5 degree steps */
#define DAC1FID21_TWINDDIR_NOT_AVAILABLE	127
			    unsigned int twindspeed;	/* meters per second */
#define DAC1FID21_TWINDSPEED_SCALE		2
#define DAC1FID21_RWINDSPEED_NOT_AVAILABLE	255
			    unsigned int rwinddir;	/* in 5 degree steps */
#define DAC1FID21_RWINDDIR_NOT_AVAILABLE	127
			    unsigned int rwindspeed;	/* meters per second */
#define DAC1FID21_RWINDSPEED_SCALE		2
#define DAC1FID21_RWINDSPEED_NOT_AVAILABLE	255
			    unsigned int mgustspeed;	/* meters per second */
#define DAC1FID21_MGUSTSPEED_SCALE		2
#define DAC1FID21_MGUSTSPEED_NOT_AVAILABLE	255
			    unsigned int mgustdir;	/* in 5 degree steps */
#define DAC1FID21_MGUSTDIR_NOT_AVAILABLE	127
			    unsigned int airtemp;	/* degress K */
#define DAC1FID21_AIRTEMP_OFFSET		223
			    unsigned humidity;		/* units of 1% */
#define DAC1FID21_HUMIDITY_NOT_VAILABLE		127
			    /* some trailing fields are missing */
			} wmo_obs;
		    };
	        } dac1fid21;
		/*** WORK IN PROGRESS ENDS HERE ***/
		/* IMO289 - Dangerous Cargo Indication */
		struct {
		    unsigned int unit;	/* Unit of Quantity */
		    unsigned int amount;	/* Amount of Cargo */
		    int ncargos;
		    struct cargo_t {
			unsigned int code;	/* Cargo code */
			unsigned int subtype;	/* Cargo subtype */
		    } cargos[28];
		} dac1fid25;
		/* IMO289 - Route info (addressed) */
		struct route_info dac1fid28;
		/* IMO289 - Text message (addressed) */
		struct {
		    unsigned int linkage;
#define AIS_DAC1FID30_TEXT_MAX	154	/* 920 bits of six-bit, plus NUL */
		    char text[AIS_DAC1FID30_TEXT_MAX];
		} dac1fid30;
		/* IMO289 & IMO236 - Tidal Window */
		struct {
		    unsigned int month;	/* Month */
		    unsigned int day;	/* Day */
		    signed int ntidals;
		    struct tidal_t {
			signed int lon;	/* Longitude */
			signed int lat;	/* Latitude */
			unsigned int from_hour;	/* From UTC Hour */
			unsigned int from_min;	/* From UTC Minute */
			unsigned int to_hour;	/* To UTC Hour */
			unsigned int to_min;	/* To UTC Minute */
#define DAC1FID32_CDIR_NOT_AVAILABLE		360
			unsigned int cdir;	/* Current Dir. Predicted */
#define DAC1FID32_CSPEED_NOT_AVAILABLE		127
			unsigned int cspeed;	/* Current Speed Predicted */
		    } tidals[3];
		} dac1fid32;
	    };
	} type6;
	/* Type 7 - Binary Acknowledge */
	struct {
	    unsigned int mmsi1;
	    unsigned int mmsi2;
	    unsigned int mmsi3;
	    unsigned int mmsi4;
	    /* spares ignored, they're only padding here */
	} type7;
	/* Type 8 - Broadcast Binary Message */
	struct {
	    unsigned int dac;       	/* Designated Area Code */
	    unsigned int fid;       	/* Functional ID */
#define AIS_TYPE8_BINARY_MAX	952	/* 952 bits */
	    size_t bitcount;		/* bit count of the data */
	    union {
		char bitdata[(AIS_TYPE8_BINARY_MAX + 7) / 8];
		/* Inland static ship and voyage-related data */
		struct {
		    char vin[8+1];	/* European Vessel ID */
		    unsigned int length;	/* Length of ship */
		    unsigned int beam;	/* Beam of ship */
		    unsigned int type;	/* Ship/combination type */
		    unsigned int hazard;	/* Hazardous cargo */
		    unsigned int draught;	/* Draught */
		    unsigned int loaded;	/* Loaded/Unloaded */
		    bool speed_q;	/* Speed inf. quality */
		    bool course_q;	/* Course inf. quality */
		    bool heading_q;	/* Heading inf. quality */
		} dac200fid10;
		/* Inland AIS EMMA Warning */
		struct {
		    unsigned int start_year;	/* Start Year */
		    unsigned int start_month;	/* Start Month */
		    unsigned int start_day;	/* Start Day */
		    unsigned int end_year;	/* End Year */
		    unsigned int end_month;	/* End Month */
		    unsigned int end_day;	/* End Day */
		    unsigned int start_hour;	/* Start Hour */
		    unsigned int start_minute;	/* Start Minute */
		    unsigned int end_hour;	/* End Hour */
		    unsigned int end_minute;	/* End Minute */
		    signed int start_lon;	/* Start Longitude */
		    signed int start_lat;	/* Start Latitude */
		    signed int end_lon;	/* End Longitude */
		    signed int end_lat;	/* End Latitude */
		    unsigned int type;	/* Type */
#define DAC200FID23_TYPE_UNKNOWN		0
		    signed int min;	/* Min value */
#define DAC200FID23_MIN_UNKNOWN			255
		    signed int max;	/* Max value */
#define DAC200FID23_MAX_UNKNOWN			255
		    unsigned int intensity;	/* Classification */
#define DAC200FID23_CLASS_UNKNOWN		0
		    unsigned int wind;	/* Wind Direction */
#define DAC200FID23_WIND_UNKNOWN		0
		} dac200fid23;
		struct {
		    char country[2+1];	/* UN Country Code */
		    signed int ngauges;
		    struct gauge_t {
			unsigned int id;	/* Gauge ID */
#define DAC200FID24_GAUGE_ID_UNKNOWN		0
			signed int level;	/* Water Level */
#define DAC200FID24_GAUGE_LEVEL_UNKNOWN		0
		    } gauges[4];
		} dac200fid24;
		struct {
		    signed int lon;	/* Signal Longitude */
		    signed int lat;	/* Signal Latitude */
		    unsigned int form;	/* Signal form */
#define DAC200FID40_FORM_UNKNOWN		0
		    unsigned int facing;	/* Signal orientation */
#define DAC200FID40_FACING_UNKNOWN		0
		    unsigned int direction;	/* Direction of impact */
#define DAC200FID40_DIRECTION_UNKNOWN		0
		    unsigned int status;	/* Light Status */
#define DAC200FID40_STATUS_UNKNOWN		0
		} dac200fid40;
		/* IMO236  - Meteorological-Hydrological data
		 * Trial message, not to be used after January 2013
		 * Replaced by IMO289 (DAC 1, FID 31)
		 */
		struct {
#define DAC1FID11_LATLON_SCALE			1000
		    int lon;			/* longitude in minutes * .001 */
#define DAC1FID11_LON_NOT_AVAILABLE		0xFFFFFF
		    int lat;			/* latitude in minutes * .001 */
#define DAC1FID11_LAT_NOT_AVAILABLE		0x7FFFFF
		    unsigned int day;		/* UTC day */
		    unsigned int hour;		/* UTC hour */
		    unsigned int minute;	/* UTC minute */
		    unsigned int wspeed;	/* average wind speed */
		    unsigned int wgust;		/* wind gust */
#define DAC1FID11_WSPEED_NOT_AVAILABLE		127
		    unsigned int wdir;		/* wind direction */
		    unsigned int wgustdir;	/* wind gust direction */
#define DAC1FID11_WDIR_NOT_AVAILABLE		511
		    unsigned int airtemp;	/* temperature, units 0.1C */
#define DAC1FID11_AIRTEMP_NOT_AVAILABLE		2047
#define DAC1FID11_AIRTEMP_OFFSET		600
#define DAC1FID11_AIRTEMP_DIV			10.0
		    unsigned int humidity;	/* relative humidity, % */
#define DAC1FID11_HUMIDITY_NOT_AVAILABLE	127
		    unsigned int dewpoint;	/* dew point, units 0.1C */
#define DAC1FID11_DEWPOINT_NOT_AVAILABLE	1023
#define DAC1FID11_DEWPOINT_OFFSET		200
#define DAC1FID11_DEWPOINT_DIV		10.0
		    unsigned int pressure;	/* air pressure, hpa */
#define DAC1FID11_PRESSURE_NOT_AVAILABLE	511
#define DAC1FID11_PRESSURE_OFFSET		-800
		    unsigned int pressuretend;	/* tendency */
#define DAC1FID11_PRESSURETREND_NOT_AVAILABLE	3
		    unsigned int visibility;	/* units 0.1 nautical miles */
#define DAC1FID11_VISIBILITY_NOT_AVAILABLE	255
#define DAC1FID11_VISIBILITY_DIV		10.0
		    int waterlevel;		/* decimeters */
#define DAC1FID11_WATERLEVEL_NOT_AVAILABLE	511
#define DAC1FID11_WATERLEVEL_OFFSET		100
#define DAC1FID11_WATERLEVEL_DIV		10.0
		    unsigned int leveltrend;	/* water level trend code */
#define DAC1FID11_WATERLEVELTREND_NOT_AVAILABLE	3
		    unsigned int cspeed;	/* surface current speed in deciknots */
#define DAC1FID11_CSPEED_NOT_AVAILABLE		255
#define DAC1FID11_CSPEED_DIV			10.0
		    unsigned int cdir;		/* surface current dir., degrees */
#define DAC1FID11_CDIR_NOT_AVAILABLE		511
		    unsigned int cspeed2;	/* current speed in deciknots */
		    unsigned int cdir2;		/* current dir., degrees */
		    unsigned int cdepth2;	/* measurement depth, m */
#define DAC1FID11_CDEPTH_NOT_AVAILABLE		31
		    unsigned int cspeed3;	/* current speed in deciknots */
		    unsigned int cdir3;		/* current dir., degrees */
		    unsigned int cdepth3;	/* measurement depth, m */
		    unsigned int waveheight;	/* in decimeters */
#define DAC1FID11_WAVEHEIGHT_NOT_AVAILABLE	255
#define DAC1FID11_WAVEHEIGHT_DIV		10.0
		    unsigned int waveperiod;	/* in seconds */
#define DAC1FID11_WAVEPERIOD_NOT_AVAILABLE	63
		    unsigned int wavedir;	/* direction in degrees */
#define DAC1FID11_WAVEDIR_NOT_AVAILABLE		511
		    unsigned int swellheight;	/* in decimeters */
		    unsigned int swellperiod;	/* in seconds */
		    unsigned int swelldir;	/* direction in degrees */
		    unsigned int seastate;	/* Beaufort scale, 0-12 */
#define DAC1FID11_SEASTATE_NOT_AVAILABLE	15
		    unsigned int watertemp;	/* units 0.1deg Celsius */
#define DAC1FID11_WATERTEMP_NOT_AVAILABLE	1023
#define DAC1FID11_WATERTEMP_OFFSET		100
#define DAC1FID11_WATERTEMP_DIV		10.0
		    unsigned int preciptype;	/* 0-7, enumerated */
#define DAC1FID11_PRECIPTYPE_NOT_AVAILABLE	7
		    unsigned int salinity;	/* units of 0.1ppt */
#define DAC1FID11_SALINITY_NOT_AVAILABLE	511
#define DAC1FID11_SALINITY_DIV		10.0
		    unsigned int ice;		/* is there sea ice? */
#define DAC1FID11_ICE_NOT_AVAILABLE		3
		} dac1fid11;
		/* IMO236 - Fairway Closed */
		struct {
		    char reason[20+1];		/* Reason For Closing */
		    char closefrom[20+1];	/* Location Of Closing From */
		    char closeto[20+1];		/* Location of Closing To */
		    unsigned int radius;	/* Radius extension */
#define AIS_DAC1FID13_RADIUS_NOT_AVAILABLE 10001
		    unsigned int extunit;	/* Unit of extension */
#define AIS_DAC1FID13_EXTUNIT_NOT_AVAILABLE 0
		    unsigned int fday;		/* From day (UTC) */
		    unsigned int fmonth;	/* From month (UTC) */
		    unsigned int fhour;		/* From hour (UTC) */
		    unsigned int fminute;	/* From minute (UTC) */
		    unsigned int tday;		/* To day (UTC) */
		    unsigned int tmonth;	/* To month (UTC) */
		    unsigned int thour;		/* To hour (UTC) */
		    unsigned int tminute;	/* To minute (UTC) */
		} dac1fid13;
	        /* IMO236 - Extended ship and voyage data */
		struct {
		    unsigned int airdraught;	/* Air Draught */
		} dac1fid15;
		/* IMO286 - Number of Persons on board */
		struct {
		    unsigned persons;	/* number of persons */
		} dac1fid16;
		/* IMO289 - VTS-generated/Synthetic Targets */
		struct {
		    signed int ntargets;
		    struct target_t {
#define DAC1FID17_IDTYPE_MMSI		0
#define DAC1FID17_IDTYPE_IMO		1
#define DAC1FID17_IDTYPE_CALLSIGN	2
#define DAC1FID17_IDTYPE_OTHER		3
			unsigned int idtype;	/* Identifier type */
			union target_id {	/* Target identifier */
			    unsigned int mmsi;
			    unsigned int imo;
#define DAC1FID17_ID_LENGTH		7
			    char callsign[DAC1FID17_ID_LENGTH+1];
			    char other[DAC1FID17_ID_LENGTH+1];
			} id;
			signed int lat;		/* Latitude */
			signed int lon;		/* Longitude */
#define DAC1FID17_COURSE_NOT_AVAILABLE		360
			unsigned int course;	/* Course Over Ground */
			unsigned int second;	/* Time Stamp */
#define DAC1FID17_SPEED_NOT_AVAILABLE		255
			unsigned int speed;	/* Speed Over Ground */
		    } targets[4];
		} dac1fid17;
		/* IMO 289 - Marine Traffic Signal */
		struct {
		    unsigned int linkage;	/* Message Linkage ID */
		    char station[20+1];		/* Name of Signal Station */
		    signed int lon;		/* Longitude */
		    signed int lat;		/* Latitude */
		    unsigned int status;	/* Status of Signal */
		    unsigned int signal;	/* Signal In Service */
		    unsigned int hour;		/* UTC hour */
		    unsigned int minute;	/* UTC minute */
		    unsigned int nextsignal;	/* Expected Next Signal */
		} dac1fid19;
		/* IMO289 - Route info (broadcast) */
		struct route_info dac1fid27;
		/* IMO289 - Text message (broadcast) */
		struct {
		    unsigned int linkage;
#define AIS_DAC1FID29_TEXT_MAX	162	/* 920 bits of six-bit, plus NUL */
		    char text[AIS_DAC1FID29_TEXT_MAX];
		} dac1fid29;
		/* IMO289 - Meteorological-Hydrological data */
		struct {
		    bool accuracy;	/* position accuracy, <10m if true */
#define DAC1FID31_LATLON_SCALE	1000
		    int lon;		/* longitude in minutes * .001 */
#define DAC1FID31_LON_NOT_AVAILABLE	(181*60*DAC1FID31_LATLON_SCALE)
		    int lat;		/* longitude in minutes * .001 */
#define DAC1FID31_LAT_NOT_AVAILABLE	(91*60*DAC1FID31_LATLON_SCALE)
		    unsigned int day;		/* UTC day */
		    unsigned int hour;		/* UTC hour */
		    unsigned int minute;	/* UTC minute */
		    unsigned int wspeed;	/* average wind speed */
		    unsigned int wgust;		/* wind gust */
#define DAC1FID31_WIND_HIGH			126
#define DAC1FID31_WIND_NOT_AVAILABLE		127
		    unsigned int wdir;		/* wind direction */
		    unsigned int wgustdir;	/* wind gust direction */
#define DAC1FID31_DIR_NOT_AVAILABLE		360
		    int airtemp;		/* temperature, units 0.1C */
#define DAC1FID31_AIRTEMP_NOT_AVAILABLE		-1024
#define DAC1FID31_AIRTEMP_DIV			10.0
		    unsigned int humidity;	/* relative humidity, % */
#define DAC1FID31_HUMIDITY_NOT_AVAILABLE	101
		    int dewpoint;		/* dew point, units 0.1C */
#define DAC1FID31_DEWPOINT_NOT_AVAILABLE	501
#define DAC1FID31_DEWPOINT_DIV		10.0
		    unsigned int pressure;	/* air pressure, hpa */
#define DAC1FID31_PRESSURE_NOT_AVAILABLE	511
#define DAC1FID31_PRESSURE_HIGH			402
#define DAC1FID31_PRESSURE_OFFSET		-799
		    unsigned int pressuretend;	/* tendency */
#define DAC1FID31_PRESSURETEND_NOT_AVAILABLE	3
		    bool visgreater;            /* visibility greater than */
		    unsigned int visibility;	/* units 0.1 nautical miles */
#define DAC1FID31_VISIBILITY_NOT_AVAILABLE	127
#define DAC1FID31_VISIBILITY_DIV		10.0
		    int waterlevel;		/* cm */
#define DAC1FID31_WATERLEVEL_NOT_AVAILABLE	4001
#define DAC1FID31_WATERLEVEL_OFFSET		1000
#define DAC1FID31_WATERLEVEL_DIV		100.0
		    unsigned int leveltrend;	/* water level trend code */
#define DAC1FID31_WATERLEVELTREND_NOT_AVAILABLE	3
		    unsigned int cspeed;	/* current speed in deciknots */
#define DAC1FID31_CSPEED_NOT_AVAILABLE		255
#define DAC1FID31_CSPEED_DIV			10.0
		    unsigned int cdir;		/* current dir., degrees */
		    unsigned int cspeed2;	/* current speed in deciknots */
		    unsigned int cdir2;		/* current dir., degrees */
		    unsigned int cdepth2;	/* measurement depth, 0.1m */
#define DAC1FID31_CDEPTH_NOT_AVAILABLE		301
#define DAC1FID31_CDEPTH_SCALE			10.0
		    unsigned int cspeed3;	/* current speed in deciknots */
		    unsigned int cdir3;		/* current dir., degrees */
		    unsigned int cdepth3;	/* measurement depth, 0.1m */
		    unsigned int waveheight;	/* in decimeters */
#define DAC1FID31_HEIGHT_NOT_AVAILABLE		31
#define DAC1FID31_HEIGHT_DIV			10.0
		    unsigned int waveperiod;	/* in seconds */
#define DAC1FID31_PERIOD_NOT_AVAILABLE		63
		    unsigned int wavedir;	/* direction in degrees */
		    unsigned int swellheight;	/* in decimeters */
		    unsigned int swellperiod;	/* in seconds */
		    unsigned int swelldir;	/* direction in degrees */
		    unsigned int seastate;	/* Beaufort scale, 0-12 */
#define DAC1FID31_SEASTATE_NOT_AVAILABLE	15
		    int watertemp;		/* units 0.1deg Celsius */
#define DAC1FID31_WATERTEMP_NOT_AVAILABLE	601
#define DAC1FID31_WATERTEMP_DIV		10.0
		    unsigned int preciptype;	/* 0-7, enumerated */
#define DAC1FID31_PRECIPTYPE_NOT_AVAILABLE	7
		    unsigned int salinity;	/* units of 0.1 permil (ca. PSU) */
#define DAC1FID31_SALINITY_NOT_AVAILABLE	510
#define DAC1FID31_SALINITY_DIV		10.0
		    unsigned int ice;		/* is there sea ice? */
#define DAC1FID31_ICE_NOT_AVAILABLE		3
		} dac1fid31;
	    };
	} type8;
	/* Type 9 - Standard SAR Aircraft Position Report */
	struct {
	    unsigned int alt;		/* altitude in meters */
#define AIS_ALT_NOT_AVAILABLE	4095
#define AIS_ALT_HIGH    	4094	/* 4094 meters or higher */
	    unsigned int speed;		/* speed over ground in deciknots */
#define AIS_SAR_SPEED_NOT_AVAILABLE	1023
#define AIS_SAR_FAST_MOVER  	1022
	    bool accuracy;		/* position accuracy */
	    int lon;			/* longitude */
	    int lat;			/* latitude */
	    unsigned int course;	/* course over ground */
	    unsigned int second;	/* seconds of UTC timestamp */
	    unsigned int regional;	/* regional reserved */
	    unsigned int dte;		/* data terminal enable */
	    //unsigned int spare;	spare bits */
	    bool assigned;		/* assigned-mode flag */
	    bool raim;			/* RAIM flag */
	    unsigned int radio;		/* radio status bits */
	} type9;
	/* Type 10 - UTC/Date Inquiry */
	struct {
	    //unsigned int spare;
	    unsigned int dest_mmsi;	/* destination MMSI */
	    //unsigned int spare2;
	} type10;
	/* Type 12 - Safety-Related Message */
	struct {
	    unsigned int seqno;		/* sequence number */
	    unsigned int dest_mmsi;	/* destination MMSI */
	    bool retransmit;		/* retransmit flag */
	    //unsigned int spare;	spare bit(s) */
#define AIS_TYPE12_TEXT_MAX	157	/* 936 bits of six-bit, plus NUL */
	    char text[AIS_TYPE12_TEXT_MAX];
	} type12;
	/* Type 14 - Safety-Related Broadcast Message */
	struct {
	    //unsigned int spare;	spare bit(s) */
#define AIS_TYPE14_TEXT_MAX	161	/* 952 bits of six-bit, plus NUL */
	    char text[AIS_TYPE14_TEXT_MAX];
	} type14;
	/* Type 15 - Interrogation */
	struct {
	    //unsigned int spare;	spare bit(s) */
	    unsigned int mmsi1;
	    unsigned int type1_1;
	    unsigned int offset1_1;
	    //unsigned int spare2;	spare bit(s) */
	    unsigned int type1_2;
	    unsigned int offset1_2;
	    //unsigned int spare3;	spare bit(s) */
	    unsigned int mmsi2;
	    unsigned int type2_1;
	    unsigned int offset2_1;
	    //unsigned int spare4;	spare bit(s) */
	} type15;
	/* Type 16 - Assigned Mode Command */
	struct {
	    //unsigned int spare;	spare bit(s) */
	    unsigned int mmsi1;
	    unsigned int offset1;
	    unsigned int increment1;
	    unsigned int mmsi2;
	    unsigned int offset2;
	    unsigned int increment2;
	} type16;
	/* Type 17 - GNSS Broadcast Binary Message */
	struct {
	    //unsigned int spare;	spare bit(s) */
#define AIS_GNSS_LATLON_DIV	600.0
	    int lon;			/* longitude */
	    int lat;			/* latitude */
	    //unsigned int spare2;	spare bit(s) */
#define AIS_TYPE17_BINARY_MAX	736	/* 920 bits */
	    size_t bitcount;		/* bit count of the data */
	    char bitdata[(AIS_TYPE17_BINARY_MAX + 7) / 8];
	} type17;
	/* Type 18 - Standard Class B CS Position Report */
	struct {
	    unsigned int reserved;	/* altitude in meters */
	    unsigned int speed;		/* speed over ground in deciknots */
	    bool accuracy;		/* position accuracy */
	    int lon;			/* longitude */
#define AIS_GNS_LON_NOT_AVAILABLE	0x1a838
	    int lat;			/* latitude */
#define AIS_GNS_LAT_NOT_AVAILABLE	0xd548
	    unsigned int course;	/* course over ground */
	    unsigned int heading;	/* true heading */
	    unsigned int second;	/* seconds of UTC timestamp */
	    unsigned int regional;	/* regional reserved */
	    bool cs;     		/* carrier sense unit flag */
	    bool display;		/* unit has attached display? */
	    bool dsc;   		/* unit attached to radio with DSC? */
	    bool band;   		/* unit can switch frequency bands? */
	    bool msg22;	        	/* can accept Message 22 management? */
	    bool assigned;		/* assigned-mode flag */
	    bool raim;			/* RAIM flag */
	    unsigned int radio;		/* radio status bits */
	} type18;
	/* Type 19 - Extended Class B CS Position Report */
	struct {
	    unsigned int reserved;	/* altitude in meters */
	    unsigned int speed;		/* speed over ground in deciknots */
	    bool accuracy;		/* position accuracy */
	    int lon;			/* longitude */
	    int lat;			/* latitude */
	    unsigned int course;	/* course over ground */
	    unsigned int heading;	/* true heading */
	    unsigned int second;	/* seconds of UTC timestamp */
	    unsigned int regional;	/* regional reserved */
	    char shipname[AIS_SHIPNAME_MAXLEN+1];		/* ship name */
	    unsigned int shiptype;	/* ship type code */
	    unsigned int to_bow;	/* dimension to bow */
	    unsigned int to_stern;	/* dimension to stern */
	    unsigned int to_port;	/* dimension to port */
	    unsigned int to_starboard;	/* dimension to starboard */
	    unsigned int epfd;		/* type of position fix deviuce */
	    bool raim;			/* RAIM flag */
	    unsigned int dte;    	/* date terminal enable */
	    bool assigned;		/* assigned-mode flag */
	    //unsigned int spare;	spare bits */
	} type19;
	/* Type 20 - Data Link Management Message */
	struct {
	    //unsigned int spare;	spare bit(s) */
	    unsigned int offset1;	/* TDMA slot offset */
	    unsigned int number1;	/* number of xlots to allocate */
	    unsigned int timeout1;	/* allocation timeout */
	    unsigned int increment1;	/* repeat increment */
	    unsigned int offset2;	/* TDMA slot offset */
	    unsigned int number2;	/* number of xlots to allocate */
	    unsigned int timeout2;	/* allocation timeout */
	    unsigned int increment2;	/* repeat increment */
	    unsigned int offset3;	/* TDMA slot offset */
	    unsigned int number3;	/* number of xlots to allocate */
	    unsigned int timeout3;	/* allocation timeout */
	    unsigned int increment3;	/* repeat increment */
	    unsigned int offset4;	/* TDMA slot offset */
	    unsigned int number4;	/* number of xlots to allocate */
	    unsigned int timeout4;	/* allocation timeout */
	    unsigned int increment4;	/* repeat increment */
	} type20;
	/* Type 21 - Aids to Navigation Report */
	struct {
	    unsigned int aid_type;	/* aid type */
	    char name[35];		/* name of aid to navigation */
	    bool accuracy;		/* position accuracy */
	    int lon;			/* longitude */
	    int lat;			/* latitude */
	    unsigned int to_bow;	/* dimension to bow */
	    unsigned int to_stern;	/* dimension to stern */
	    unsigned int to_port;	/* dimension to port */
	    unsigned int to_starboard;	/* dimension to starboard */
	    unsigned int epfd;		/* type of EPFD */
	    unsigned int second;	/* second of UTC timestamp */
	    bool off_position;		/* off-position indicator */
	    unsigned int regional;	/* regional reserved field */
	    bool raim;			/* RAIM flag */
	    bool virtual_aid;		/* is virtual station? */
	    bool assigned;		/* assigned-mode flag */
	    //unsigned int spare;	unused */
	} type21;
	/* Type 22 - Channel Management */
	struct {
	    //unsigned int spare;	spare bit(s) */
	    unsigned int channel_a;	/* Channel A number */
	    unsigned int channel_b;	/* Channel B number */
	    unsigned int txrx;		/* transmit/receive mode */
	    bool power;			/* high-power flag */
#define AIS_CHANNEL_LATLON_DIV	600.0
	    union {
		struct {
		    int ne_lon;		/* NE corner longitude */
		    int ne_lat;		/* NE corner latitude */
		    int sw_lon;		/* SW corner longitude */
		    int sw_lat;		/* SW corner latitude */
		} area;
		struct {
		    unsigned int dest1;	/* addressed station MMSI 1 */
		    unsigned int dest2;	/* addressed station MMSI 2 */
		} mmsi;
	    };
	    bool addressed;		/* addressed vs. broadast flag */
	    bool band_a;		/* fix 1.5kHz band for channel A */
	    bool band_b;		/* fix 1.5kHz band for channel B */
	    unsigned int zonesize;	/* size of transitional zone */
	} type22;
	/* Type 23 - Group Assignment Command */
	struct {
	    int ne_lon;			/* NE corner longitude */
	    int ne_lat;			/* NE corner latitude */
	    int sw_lon;			/* SW corner longitude */
	    int sw_lat;			/* SW corner latitude */
	    //unsigned int spare;	spare bit(s) */
	    unsigned int stationtype;	/* station type code */
	    unsigned int shiptype;	/* ship type code */
	    //unsigned int spare2;	spare bit(s) */
	    unsigned int txrx;		/* transmit-enable code */
	    unsigned int interval;	/* report interval */
	    unsigned int quiet;		/* quiet time */
	    //unsigned int spare3;	spare bit(s) */
	} type23;
	/* Type 24 - Class B CS Static Data Report */
	struct {
	    char shipname[AIS_SHIPNAME_MAXLEN+1];	/* vessel name */
	    enum {
		both,
		part_a,
		part_b,
	    } part;
	    unsigned int shiptype;	/* ship type code */
	    char vendorid[8];		/* vendor ID */
	    unsigned int model;		/* unit model code */
	    unsigned int serial;	/* serial number */
	    char callsign[8];		/* callsign */
	    union {
		unsigned int mothership_mmsi;	/* MMSI of main vessel */
		struct {
		    unsigned int to_bow;	/* dimension to bow */
		    unsigned int to_stern;	/* dimension to stern */
		    unsigned int to_port;	/* dimension to port */
		    unsigned int to_starboard;	/* dimension to starboard */
		} dim;
	    };
	} type24;
	/* Type 25 - Addressed Binary Message */
	struct {
	    bool addressed;		/* addressed-vs.broadcast flag */
	    bool structured;		/* structured-binary flag */
	    unsigned int dest_mmsi;	/* destination MMSI */
	    unsigned int app_id;        /* Application ID */
#define AIS_TYPE25_BINARY_MAX	128	/* Up to 128 bits */
	    size_t bitcount;		/* bit count of the data */
	    char bitdata[(AIS_TYPE25_BINARY_MAX + 7) / 8];
	} type25;
	/* Type 26 - Addressed Binary Message */
	struct {
	    bool addressed;		/* addressed-vs.broadcast flag */
	    bool structured;		/* structured-binary flag */
	    unsigned int dest_mmsi;	/* destination MMSI */
	    unsigned int app_id;        /* Application ID */
#define AIS_TYPE26_BINARY_MAX	1004	/* Up to 128 bits */
	    size_t bitcount;		/* bit count of the data */
	    char bitdata[(AIS_TYPE26_BINARY_MAX + 7) / 8];
	    unsigned int radio;		/* radio status bits */
	} type26;
	/* Type 27 - Long Range AIS Broadcast message */
	struct {
	    bool accuracy;		/* position accuracy */
	    bool raim;			/* RAIM flag */
	    unsigned int status;	/* navigation status */
#define AIS_LONGRANGE_LATLON_DIV	600.0
	    int lon;			/* longitude */
#define AIS_LONGRANGE_LON_NOT_AVAILABLE	0x1a838
	    int lat;			/* latitude */
#define AIS_LONGRANGE_LAT_NOT_AVAILABLE	0xd548
	    unsigned int speed;		/* speed over ground in deciknots */
#define AIS_LONGRANGE_SPEED_NOT_AVAILABLE 63
	    unsigned int course;	/* course over ground */
#define AIS_LONGRANGE_COURSE_NOT_AVAILABLE 511
	    bool gnss;			/* are we reporting GNSS position? */
	} type27;
    };
};

#include "aivdm_decode.h"

#endif

