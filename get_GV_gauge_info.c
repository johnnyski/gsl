
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "gsl.h"


#define KM_PER_DEG 111.2
#if defined (__linux)
#undef PI
#endif
#define PI 3.14159265


/***********************************************************/
/*                                                         */
/*             get_gauge_networks_for_radar_site           */
/*                                                         */
/***********************************************************/
int get_gauge_networks_for_radar_site(char *top_dir, char *radar_id, 
				      char *networks[], int *number_gnet,
				      float *radarLat, float *radarLon)
{
	/*
		 Retrieves a list of gauge network names associated with
		 a given radar, as well as the radarLat and radarLon.

		 Returns: 0, if success.
		         -1, otherwise.
	*/

	FILE     *fp;
	char     buffy[1000], radar_file[300];
	char     radar[8];
	char     gnet[8], gv_site[8];
	int      net_index, verbose=1;
	float    rlat, rlon;
	strcpy(radar_file, top_dir);   
	strcat(radar_file, "/sitelist/");
	strcat(radar_file,"radar.dat"); 

	if((fp = fopen(radar_file,"r"))== NULL){
		fprintf(stderr,"Cannot open %s\n", radar_file);
		return(-1);
	}

	/* Read in each line from the file 'radar.dat'.
	 * Understands comments w/ # in the first column.  Headers must
	 * be comments.
	 */
	net_index = 0;
	while (fgets(buffy, sizeof(buffy), fp) != NULL){
	  if (buffy[0] == '#') continue; /* Skip commented lines. */
	  sscanf(buffy, "%s %s %s %f %f", gv_site, gnet, radar, &rlat, &rlon);
	  if(verbose)
	    fprintf(stderr, "%s %s %s %.2f %.2f\n", gv_site,
		    gnet, radar, rlat, rlon);
	  if(strcmp(radar_id, radar)==0){
	    if (verbose)
	      fprintf(stderr,"  Found network: %s in %s\n", gnet, radar_id);
	    networks[net_index] = (char *) strdup(gnet);
	    if (net_index == 0){
	      *radarLat = rlat;
	      *radarLon = rlon;
	    }
	    net_index++;
	    if (net_index >= MAX_GAUGE_NETWORKS) return(-1);
	  }
	} /* end while (fscanf... */
	*number_gnet = net_index;
	fclose(fp);
	return(0);
}

/***********************************************************/
/*                                                         */
/*                gauge_range_azimuth (Brad Fisher)        */
/*                                                         */
/***********************************************************/
/*
     *** This function superceeded by Eyals function  ***
     (See Eyals function below).

void gauge_range_azimuth(float RLAT, float RLON, float GLAT, float GLON,
												 float *GRANGE, float *GAZ)
{
	double DLON, DLAT, RLAT_RAD, RLON_RAD, GLAT_RAD, RLAT_DIST, AVG_LAT;
  double RLON_DIST, AZ;

	DLON      = GLON - RLON;
	DLAT      = GLAT - RLAT;
	RLAT_RAD  = (PI/180.) * DLAT;
	RLON_RAD  = (PI/180.) * DLON;
	GLAT_RAD  = GLAT * (PI/180.);
	RLAT_DIST = DLAT * KM_PER_DEG;
	AVG_LAT   = .5 * (RLAT_RAD + GLAT_RAD);
	RLON_DIST = cos(AVG_LAT) * DLON * KM_PER_DEG;
	AZ        = atan2(RLON_DIST, RLAT_DIST) * (180./PI) ;
	if (AZ < 0) *GAZ = 360.0 + AZ;
	else *GAZ = AZ;

	*GRANGE = sqrt( pow(RLON_DIST, 2.0) + pow(RLAT_DIST, 2.0) );
}
*/

/***********************************************************/
/*                                                         */
/*                gauge_range_azimuth  (EYAL AMITAI)       */
/*                                                         */
/***********************************************************/
void gauge_range_azimuth(float radar_lat, float radar_lon,
						 float point_lat, float point_lon,
						 float *range, float *azim)
{
  /*
	c **********************************************************************
	C *  This subroutine gets any lat & long point and calculates the RANGE*
	C *  & AZ from the radar to the given point                            *
	C **********************************************************************
	C *    PROGRAM WRITTEN BY: EYAL AMITAI                                 *
	c *                        JCET/UMBC - GSFC/NASA                       *
	c **********************************************************************
	C *    PROGRAM WAS LAST MODIFIED:   NOV 1, 1999                        *
	c **********************************************************************
	*/
        double AZ,LA1,LA2,LON1,LON2,S;

        LA2=point_lat/57.29578;
        LA1=radar_lat/57.29578;
        LON2=point_lon/57.29578;
        LON1=radar_lon/57.29578;

        S = sin(LA1)*sin(LA2)+cos(LA1)*cos(LA2)*cos(LON2-LON1);
        if(S > 1.0) S=1.0;
        if(S < -1.0) S=-1.0;

        *range=6378*acos(S);

        if(LA2 == LA1)
		{
          if(LON2-LON1 >= 0)
            AZ=90.0;
          else 
            AZ=270.0;
		}
        else
		{
          AZ = 57.296*atan((cos((LA1+LA2)/2)*(LON2-LON1))/(LA2-LA1));
          if(LA2-LA1 < 0)
            AZ=AZ+180.0;
          else
            if(LON2-LON1 < 0) AZ=AZ+360.0;
		}
        *azim=AZ;
}

/***********************************************************/
/*                                                         */
/*                 get_gauge_sites_info                    */
/*                                                         */
/***********************************************************/
Gauge_list *get_gauge_sites_info(char *top_dir, char *gnet,
								 float radarLat, float radarLon) 
{
	/* Get the gauge info for each gauge in gnet, and store in
		 structure Gauge_list.

		 Returns: gauge_list, if success.
		          NULL, otherwise.
	 */
	int     gauge_index;
	char    sitenumber[5], name[32];
	char    sitefile[120];
	char    line[128];
	float   latd, lond, latm, lonm, lats, lons;
	Gauge_list *glist;
	FILE    *fp;

	glist = (Gauge_list *) calloc(1, sizeof(Gauge_list));
	if (glist == NULL) {
	  perror("calloc glist");
	  return NULL;
	}
	/* Allocate some memory for glist */
	glist->ngauges = MAX_NETWORK_GAUGES;  /* Currently 300 */
	glist->g = (Gauge_info *) calloc(glist->ngauges, sizeof(Gauge_info));

	/* Construct sitelist filename and path for extracting gauge info */
	strcpy(sitefile, top_dir);
	strcat(sitefile,"/sitelist/");
	strcat(sitefile, gnet);
	strcat(sitefile,"_loc.dat");	

	if((fp = fopen(sitefile,"r"))== NULL) {
		fprintf(stderr,"Cannot open sitelist %s\n", sitefile);
		return(NULL);
	}

	gauge_index = 0;
	/* Extract gauge info from sitelist file. */
	while(fgets(line,128,fp)){
		if (sscanf(line,"%s %s %f %f %f %f %f %f", sitenumber,name,
							 &lond,&lonm,&lons,&latd,&latm,&lats) != 8) continue;
		glist->g[gauge_index].site_id = (char *) strdup(sitenumber);
		glist->g[gauge_index].name    = (char *) strdup(name);
		if (latd >= 0)
		  glist->g[gauge_index].lat = latd + latm/60. + lats/3600.;
		else  /* latd < 0 */
		  glist->g[gauge_index].lat = latd - latm/60. - lats/3600.;
		if (lond >= 0)
		  glist->g[gauge_index].lon = lond + lonm/60. + lons/3600.;
		else  /* lond < 0 */
		  glist->g[gauge_index].lon = lond - lonm/60. - lons/3600.;
		/* Compute gauge range & azimuth, and store in Gauge_list node. */
		gauge_range_azimuth(radarLat, radarLon, glist->g[gauge_index].lat,
							glist->g[gauge_index].lon, &glist->g[gauge_index].range,
							&glist->g[gauge_index].azimuth);
		gauge_index++;
		if (gauge_index >= glist->ngauges){
			fprintf(stderr, "get_site_info(): Too many gauges: %d\n",
							gauge_index);
			return(NULL);
		}
	} /* end while(fgets... */
	glist->ngauges = gauge_index;
	return glist;
}



void free_gauge_list(Gauge_list *glist)
{
  if (glist == NULL) return;
  if (glist->g) free(glist->g);
  free(glist);
}


