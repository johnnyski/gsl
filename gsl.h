/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Gauge Software Library.
    Copyright (C) 1996  John Merritt, Mike Kolander of
		                    Applied Research Corporation,
                        Landover, Maryland, a NASA/GSFC on-site contractor.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/******************************************************************/
/*
 * The Gauge Software Library.  Version pre-0.3
 *
 * The top_level structure defined by this library is the 'Gauge_complex'.
 *
 * In general, a GSL 'Gauge_complex' structure contains any amount of
 * data from any number of raingauges from any number of raingauge
 * networks located at one radar site. Ditto for disdrometer data.
 *
 *
 ********************************************************************/
#ifndef __GSL_H__
#define __GSL_H__ 1

#define GSL_VERSION_STR "gsl-v1.4"
#define MAX_GAUGE_NETWORKS 16   /* Max networks at one radar site. */
#define MAX_NETWORK_GAUGES 300  /* Max gauges per gauge_network. */

#define OK     0
#define ABORT -1
#define RAINGAUGE 26
#define DISDROGAUGE 27

/* Structures to hold gauge info from site list */
typedef struct {
	char  *site_id;
	char  *name;
	float lat;
	float lon;
  float azimuth; 	/* Relative to primary radar */
  float range;   	/* Relative to primary radar */
} Gauge_info;

typedef struct {
	Gauge_info *g;
	int ngauges;
} Gauge_list;

/* Need to include:
   1. Example strings for 'names', 'types', etc.
   2. Units for values.
   */

typedef struct {
  int   	year;		/* Year (4-digits: 19xx, or 20xx)*/
  int   	jday; 		/* Julian day from begining of the year (int 0-365) */
  int		month;      /* Month (int 0-11) */
  int		day;        /* Day of month (int 0-30) */
  int   	hour;		/* Hour of the day (int 0-23) */
  int   	minute;     /* Minute of the day (int 0-59) */
  float 	sec; 		/* Second (float) */
} Gauge_time;

typedef struct {
  Gauge_time  	time;
/*  float 	rate;	*/	/* Rain Rate [mm hr-1] (float) */
	float *value;     /* value[bin] , where: 
											      bin = 0  , for raingauges. (Rain rate)
											 0 <= bin < 20 , for disdrometer Drop Size Distribution
										*/
} Gauge_record;

typedef struct {
  char    *network; /* Network name: KSC, STJ ... (Also in Network header */
  char    *gv_site; /* Can be HSTN, MELB, DARW, KWAJ, THOM, GUAM,
					 *        ISBN, SAOP, and TWWF.
                     */
  char    *product_id; /* Can be GMIN, 2A56, 2A57. */
  int  		number; 	/* Number of this gauge (from header) */
  char  	*name; 		/* Name of the gauge (from header) */
  char  	*type; 		/* "TIP", "HAN", "REC", "ORG", "DSD" (from header) */
  float		resolution;	/* Resolution in minutes  (from (header) */
  float 	lat;  		/* Latitude in degrees (from (header) */
  float 	lon;  		/* Longitude in degrees (from (header) */
  char     *radar;      
  float  	azimuth; 	/* Relative to primary radar */
  float  	range;   	/* Relative to primary radar */
  float   elevation;  /* Elevation of gauge above msl (m) */
  int    	nobs;    	/* Number of observations (values) in 'record' array */
	int     nbin;    /* Number of bins per observation:
											 nbin = 1, for raingauges.
											      = 20, for disdrometers.
										*/
} Gauge_header;

typedef struct {
  Gauge_header 	h;
  Gauge_record 	*record; /* 0..< h.nobs */
} Gauge;

typedef struct {
  char 		*name;       	/* Name of this network (from individual header */
  char 		*location;   	/* Network location FLA, TEX, DAR, etc */
  char  	*type; 		/* "TIP", "HAN", "REC", "ORG", "DSD" (from header) */
  int 		ngauge;      	/* Number of gauges in gauge array. */
} Gauge_network_header;

typedef struct {
  Gauge_network_header h;
  Gauge **gauge; 			/* gauge[0..ngauge-1]. */
} Gauge_network;

typedef struct {
  char 			*radarSite; /* Radar site to which this gauge_complex is
														attached. MELB, KWAJ, DARW, etc. */
  int 			nnet;	       /* Number of gauge networks at this radar site */
} Gauge_complex_header;

typedef struct {
	Gauge_complex_header h;
  Gauge_network **net; 	 /* net[0..nnet-1]. */
} Gauge_complex;

typedef struct {
  Gauge_header h; /* h.nobs == 1 */
  float ob;       /* The observation. */
} Gauge_measurement;

typedef struct {
  Gauge_time time; /* The time of this measurement. */
  /* 'vals' is a list of gauge 'ids' and measurements for
   * the time; one measurement per time.
   */
  Gauge_measurement *val; /* 0..<Gauge_network->h.ngauge */
} Gauge_measurement_at_time;


/* Read gauge/disdrometer raw data files */
Gauge *Gread_disdro_gauge(char *infile);
Gauge *Gread_gmin(char *infile);

/* Read/write HDF files. */
int Gauge_complex_to_hdf(Gauge_complex *gcomplex, char *hdffile);
Gauge_complex *Ghdf_to_gauge_complex(char *hdffile);

/* Memory allocation */
Gauge            *Gnew_gauge(int nobs, int nbin);
Gauge_network    *Gnew_gauge_network(int ngauge);
Gauge_complex    *Gnew_gauge_complex(int nnet);
Gauge            *Gcopy_gauge(Gauge *g);

/* Memory deallocation. */
void Gfree_gauge(Gauge *gauge);
void Gfree_gauge_network(Gauge_network *network);
void Gfree_gauge_complex(Gauge_complex *gc);

/* Clear memory. */
Gauge            *Gclear_gauge(Gauge *gauge);
Gauge_network    *Gclear_gauge_network(Gauge_network *network);
Gauge_complex *Gclear_gauge_complex(Gauge_complex *site);

/* Miscellaneous */
Gauge_complex *Gconstruct_gauge_complex(int nfile, char **file,
										int instrument);
void print_network(Gauge_network *gnet);
char *find_gauge_radarSite(char *netName);
Gauge_network *find_network_in_gauge_complex(Gauge_complex *gc, 
											 char *netName);

/* Gauge info */
int get_gauge_networks_for_radar_site(char *top_dir, char *radar_id, 
																			char *networks[], int *number_gnet,
																			float *radarLat, float *radarLon);
Gauge_list *get_gauge_sites_info(char *top_dir, char *gnet,
																 float radarLat, float radarLon);
void free_gauge_list(Gauge_list *glist);

#endif
