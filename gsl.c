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

/******************************************************************

	Miscellaneous GSL library functions which operate on GSL objects
	defined in file 'gsl.h'.

	A GSL 'Gauge_complex' structure contains data from all raingauges 
	from all raingauge networks located at one radar site.
	Alternatively, it may contain data from all disdrometer networks
	located at one radar site.

  -----------------------------------------------------------------
		mike.kolander@trmm.gsfc.nasa.gov
		(301) 286-1540
*******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gsl.h"

/* static int julian(int mo, int day, int year); */
static void ymd(int jday, int yy, int *mm, int *dd);


/***********************************************************************/
/*                                                                     */
/*                                                                     */
/*                            julian                                   */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static int daytab[2][13] = {
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

/*
static int julian(int mo, int day, int year)
{
  int leap;

  leap = (year%4 == 0 && year%100 != 0) || year%400 == 0;
  return day + daytab[leap][mo-1];
}
*/
static void ymd(int jday, int yy, int *mm, int *dd)
{
  /*  Input: jday, yy */
  /* Output: mm, dd */
  int leap;
  int i;

  leap = (yy%4 == 0 && yy%100 != 0) || yy%400 == 0;
  for (i=0; daytab[leap][i]<jday; i++) continue;
  *mm = i;
  i--;
  *dd = jday - daytab[leap][i];
}

/*************************************************************/
/*                                                           */
/*                      Gprint_network                       */
/*                                                           */
/*************************************************************/
void Gprint_network(Gauge_network *gnet)
{
  int i;
  Gauge *g;

  if (gnet == NULL) return;
  for (i = 0; i<gnet->h.ngauge; i++) {
	g = gnet->gauge[i];
	printf("------------< Gauge # %d of %d>------------\n", i+1,gnet->h.ngauge);
	printf("  Name: %s\n", g->h.name);
	printf("  Number: %d\n", g->h.number);
	printf("  Type: %s, Lat %f, Lon %f\n", g->h.type, g->h.lat, g->h.lon);
  }
}

/*************************************************************/
/*                                                           */
/*                 Gfree_gauge                               */
/*                                                           */
/*************************************************************/
void Gfree_gauge(Gauge *g)
{
  if (g != NULL)
	{
		if (g->record != NULL)
		{
			if (g->record->value != NULL)
			  free(g->record->value);
			free(g->record);
		}
		free(g);
	}
}

/*************************************************************/
/*                                                           */
/*                 Gnew_gauge                                */
/*                                                           */
/*************************************************************/
Gauge *Gnew_gauge(int nobs, int nbin)
{
	/* Note nbin = 1,  for standard raingauges
		           = 20, for disdrometers.
	*/
	int j;
  Gauge *g;

  g = (Gauge *)calloc(1, sizeof(Gauge));
  if (g==NULL) perror("Gnew_gauge -- Allocating g");
  g->h.nobs = nobs;
	g->h.nbin = nbin;
  g->record = (Gauge_record *)calloc(nobs, sizeof(Gauge_record));
  if (g->record==NULL) perror("Gnew_gauge -- Allocating g->record");
	/* Allocate data storage space for 'nobs' observations. */
	g->record->value = (float *)calloc(nobs*nbin, sizeof(float));
	if (g->record->value==NULL)
	  perror("Gnew_gauge -- Allocating g->record->value");

	/* Fill in all the record.value pointers. */
	for (j=1; j<nobs; j++)
	  g->record[j].value = (float *)(g->record->value + j*nbin);

  return g;
}

/*************************************************************/
/*                                                           */
/*                 Gfree_gauge_network                       */
/*                                                           */
/*************************************************************/
void Gfree_gauge_network(Gauge_network *net)
{
	int j;
	
	if (net != NULL)
	{
		if (net->gauge != NULL)
		{
			for (j=0; j<net->h.ngauge; j++)
			  Gfree_gauge(net->gauge[j]);
			free(net->gauge);
		}
		free(net);
	}
}

/*************************************************************/
/*                                                           */
/*                 Gnew_gauge_network                        */
/*                                                           */
/*************************************************************/
Gauge_network *Gnew_gauge_network(int ngauge)
{
  Gauge_network *gnet;
  gnet = (Gauge_network *) calloc(1, sizeof(Gauge_network));
  if (gnet == NULL)
	{
		perror("Gnew_gauge_network -- gnet");
		return NULL;
  }
  gnet->gauge = (Gauge **)calloc(ngauge, sizeof(Gauge *));
  if (gnet->gauge == NULL)
	{
		perror("Gnew_gauge_network -- gnet->gauge");
		return NULL;
  }
  return gnet;
}

/*************************************************************/
/*                                                           */
/*                  Gfree_gauge_complex                      */
/*                                                           */
/*************************************************************/
void Gfree_gauge_complex(Gauge_complex *gc)
{
	int j;
	
	if (gc != NULL)
	{
	  if (gc->net != NULL)
		{
	    for (j=0; j<gc->h.nnet; j++)
	      Gfree_gauge_network(gc->net[j]);
			free(gc->net);
		}
		free(gc);
	}
}

/*************************************************************/
/*                                                           */
/*                  Gnew_gauge_complex                       */
/*                                                           */
/*************************************************************/
Gauge_complex *Gnew_gauge_complex(int nnet)
{
	Gauge_complex *gc;
	
	gc = (Gauge_complex *)calloc(1, sizeof(Gauge_complex));
	if (gc == NULL) return(NULL);
	gc->net = (Gauge_network **)calloc(nnet, sizeof(Gauge_network *));
	if (gc->net == NULL) return(NULL);
	return(gc);
}

/*************************************************************/
/*                                                           */
/*                       Gprint_gauge                        */
/*                                                           */
/*************************************************************/
void Gprint_gauge(Gauge *g)
{
  int i, j;

  if (g == NULL) {
	printf("Gprint_gauge: g == NULL\n");
	return;
  }

  printf("%4.4d %s %s %f %f %f %f\n",
				 g->h.number, g->h.name, g->h.type,
				 g->h.lat, g->h.lon,
				 g->h.range, g->h.azimuth);

  for (i=0; i<g->h.nobs; i++)
	{
		printf("%2.2d %2d (%2.2d/%2.2d/%2.2d) %2.2d %2.2d %.1f  ",
					 g->record[i].time.year, g->record[i].time.jday,
					 g->record[i].time.month, g->record[i].time.day,
					 g->record[i].time.year,
					 g->record[i].time.hour, g->record[i].time.minute,
					 g->record[i].time.sec);
	  for (j=0; j<g->h.nbin; j++)
		  printf("%.1f ", g->record[i].value[j]);
		printf("\n");
	}
}

/*************************************************************/
/*                                                           */
/*                     copy_to_larger_obs                    */
/*                                                           */
/*************************************************************/
Gauge *copy_to_larger_obs(Gauge *g_old, int n)
{
  Gauge *g;
  if (g_old == NULL) return NULL;
/*  g = Gnew_gauge(n); */
  g = Gnew_gauge(n, 20);
  g->h = g_old->h;  /* Overwrites h.nobs.  See next line. */
  g->h.nobs = n;    /* We'll check if this is enough, during read. */

  /* Copy */
  memcpy(g->record, g_old->record, sizeof(Gauge_record)*g_old->h.nobs);
  free(g_old->record);
  free(g_old);
  return g;
}
  
/*************************************************************/
/*                                                           */
/*                       Gread_gmin                          */
/*                                                           */
/*************************************************************/
Gauge *Gread_gmin(char *infile)
{
	/* This function is nearly identical to function 'Gread_disdro_gauge'.
		 Since I don't know if the finalized formats of gmin and disdro 
		 files will be the same, it's wise for now to keep 2 distinct
		 functions.
  */
  FILE *fp;
  Gauge *g;
  char name[16];
  char type[16];
  char network[16];
  char gv_site[16];
  char product[16];
  int n, yy, jday, hh, mm, ss;
  float ob;
  char radar[16];

 /* The default amount asked for is 2500 observations.  If this is
  * not enough, then 2500 more are allocated -- the original is
  * saved and new measurements are appended.  See copy_to_larger_obs.
  */
  int maxobs = 2500;

  fp = fopen(infile, "r");
  if (fp==NULL)
	{
		perror(infile);
		return NULL;
  }

  g = Gnew_gauge(maxobs, 1);
	if (g == NULL) return(NULL);
	
  /* Read the header record. */
  fscanf(fp, "%s %s %s %d %s %s %f %f %f %s %f %f %f \n",
		 product, gv_site,
				 network, &g->h.number, name, type, &g->h.resolution,
				 &g->h.lat, &g->h.lon, radar,
				 &g->h.range, &g->h.azimuth, &g->h.elevation);

  g->h.radar= (char *) strdup(radar);
  g->h.name = (char *)strdup(name);
  g->h.type = (char *)strdup(type);
  g->h.network    = (char *)strdup(network);
  g->h.product_id = (char *)strdup(product);
  g->h.gv_site    = (char *)strdup(gv_site);

  n = 0;
  while(fscanf(fp, "%d %d %d %d %d %f\n",
			   &yy, &jday, &hh, &mm, &ss, &ob) != EOF) {
	g->record[n].time.year   = yy;
	g->record[n].time.jday   = jday;
	ymd(jday, yy, &g->record[n].time.month, &g->record[n].time.day);
	g->record[n].time.hour   = hh;
	g->record[n].time.minute = mm;
	g->record[n].time.sec    = ss;
	g->record[n].value[0] = ob;
	n++;
	if (n >= g->h.nobs)
	  g = copy_to_larger_obs(g, g->h.nobs + maxobs);
  }
  fclose(fp);
  g->h.nobs = n;
  return g;
}

/*************************************************************/
/*                                                           */
/*                       Gread_disdro_gauge                  */
/*                                                           */
/*************************************************************/
Gauge *Gread_disdro_gauge(char *infile)
{
  FILE *fp;
  Gauge *g;
  char name[16];
  char type[16];
  char network[16];
  int val;
  int k, n, yy, jday, hh, mm;
 /* The default amount asked for is 2500 observations.  If this is
  * not enough, then 2500 more are allocated -- the original is
  * saved and new measurements are appended.  See copy_to_larger_obs.
  */
  int maxobs = 2500;

  fp = fopen(infile, "r");
  if(fp==NULL)
	{
		perror(infile);
		return NULL;
  }

  g = Gnew_gauge(maxobs, 20);
	if (g == NULL) return(NULL);
	
  /* Read the header record. */
  fscanf(fp, "%d %s %s %s %f %f %f %f %f %f\n",
				 &g->h.number, name, network, type, &g->h.resolution,
				 &g->h.lat, &g->h.lon,
				 &g->h.elevation,
				 &g->h.range, &g->h.azimuth);

  g->h.name = (char *) strdup(name);
  g->h.type = (char *) strdup(type);
  g->h.network = (char *) strdup(network);

  n = 0;
  while(fscanf(fp, "%d %d %2d%2d\n", &yy, &jday, &hh, &mm) != EOF)
	{
		g->record[n].time.year   = yy;
		g->record[n].time.jday   = jday;
		ymd(jday, yy, &g->record[n].time.month, &g->record[n].time.day);
		g->record[n].time.hour   = hh;
		g->record[n].time.minute = mm;
		g->record[n].time.sec    = 0.0;
		for (k=0; k<20; k++)
		{
		  if (fscanf(fp, "%d", &val) == EOF)
			  break;
			g->record[n].value[k] = (float)val;
		}
	
		n++;
		if (n >= g->h.nobs)
	    g = copy_to_larger_obs(g, g->h.nobs + maxobs);
  }
  fclose(fp);
  g->h.nobs = n;
  return g;
}

/*************************************************************/
/*                                                           */
/*                 Gconstruct_gauge_complex                  */
/*                                                           */
/*************************************************************/
Gauge_complex *Gconstruct_gauge_complex(int nfile, char **file,
																				int instrument)
{
	/* Reads raingauge (disdrometer) data from all input data files into
		 a GSL Gauge_complex structure.

		 Checks that all included gauges belong to the same radar site.
		 NOTE: A GSL 'Gauge_complex' structure contains any amount of
		 data from any number of raingauges from any number of raingauge
		 networks located at ONE radar site. Ditto for disdrometer data.

		 Returns: gauge_complex if success.
		          NULL if fails.
  */
	char *radarSite;
	int j;
	Gauge_complex *gcomplex;
	Gauge_network *gnet;
	Gauge *g;
	
	/* Create and initialize the GSL gauge_complex structure. */
	gcomplex = (Gauge_complex *)Gnew_gauge_complex(MAX_GAUGE_NETWORKS);
	if (gcomplex == NULL)
	{
		fprintf(stderr, "**Error allocating gauge_complex.\n");
		return(NULL);
	}
	gcomplex->h.radarSite = (char *) strdup("N/A");
	gcomplex->h.nnet = 0;
	
	/* Loop to read each raingauge or disdrometer data file into the GSL
		 gauge_complex. */
	for (j=0; j<nfile; j++)
	{
		fprintf(stderr, "Reading gauge file: %s\n", file[j]);
		if (instrument == RAINGAUGE) g = (Gauge *)Gread_gmin(file[j]);
		else g = (Gauge *)Gread_disdro_gauge(file[j]);
		if (g == NULL)
		{
		  fprintf(stderr, "** Error reading gauge file: %s\n", file[j]);
			exit(0);
			continue;
		}
		/* Find the network to which this gauge belongs in the gauge_complex
			 structure. */
		gnet = (Gauge_network *)find_network_in_gauge_complex(gcomplex,
																													 g->h.network);
		/* If no such net, create a new network and add it to the complex. */
		if (gnet == NULL)
		{
			if (gcomplex->h.nnet == MAX_GAUGE_NETWORKS)
			{
				fprintf(stderr, "*** Exceeded max number of gauge networks: %d\n",
								MAX_GAUGE_NETWORKS);
				Gfree_gauge_complex(gcomplex);
				return(NULL);
			}
			fprintf(stderr, "*** Creating GSL network: %s\n", g->h.network);
			gnet = (Gauge_network *)Gnew_gauge_network(MAX_NETWORK_GAUGES);
			gnet->h.name = g->h.network;
			gnet->h.type =  g->h.type;
			gnet->h.ngauge = 0;
			gcomplex->net[gcomplex->h.nnet] = gnet; /* Add net to complex */ 
			gcomplex->h.nnet++;
			/* Retrieve from a database file the name of the radar site to
				 which this new gauge_network is attached. Then check that
				 this gauge_network belongs in this gauge_complex. */
			radarSite = (char *) strdup(find_gauge_radarSite(gnet->h.name));
			if (strcmp(radarSite, "???") == 0)
			{
				Gfree_gauge_complex(gcomplex);
				return(NULL);
			}
			
			/* Does this gauge's radar site match this complex's radar site? */
			if (strcmp(radarSite, gcomplex->h.radarSite) != 0)
			{
			  if (j == 0)  /* First gauge file establishes radar site. */
				  gcomplex->h.radarSite = radarSite;
				else /* This gauge(network) does not belong to this gauge_complex */
				{
					fprintf(stderr, "**Gauge: %s from network: %s from radarSite: %s\n",
									g->h.name, g->h.network, radarSite);
					fprintf(stderr, "does not belong to this gauge_complex from : %s\n",
									gcomplex->h.radarSite);
					Gfree_gauge_complex(gcomplex);
					return(NULL);
				}
			} /* end if (strcmp(radarSite, ... */
		} /* end if (gnet == NULL) */

		if (gnet->h.ngauge == MAX_NETWORK_GAUGES)
		{
			fprintf(stderr, "*** Exceeded max number of gauges: %d in network: %s\n",
							MAX_NETWORK_GAUGES, gnet->h.name);
			Gfree_gauge_complex(gcomplex);
			return(NULL);
		}
		gnet->gauge[gnet->h.ngauge] = g; /* Add gauge to network. */
		gnet->h.ngauge++;
	} /* for (j=0; j<nfile; j++) */

	return(gcomplex);
}

/*************************************************************/
/*                                                           */
/*                 Gcopy_gauge                               */
/*                                                           */
/*************************************************************/
Gauge *Gcopy_gauge(Gauge *g)
{
  Gauge *newg;

  if (g == NULL) return NULL;
  newg = (Gauge *)calloc(1, sizeof(Gauge));
  if (newg == NULL) {
	perror("Gcopy_gauge");
	return NULL;
  }
  newg->h = g->h;
  /* Explicitly, copy some strings.  Using a copy of the pointer
   * seems ok, but, this will give a true copy.
   */
  newg->h.name = (char *) strdup(newg->h.name);
  newg->h.type = (char *) strdup(newg->h.type);

  /* Allocate the space for the observations. */
  newg->record = (Gauge_record *)calloc(newg->h.nobs, sizeof(Gauge_record));
  if (newg->record == NULL) {
	perror("Gcopy_gauge, ->record");
	return NULL;
  }

  memcpy(newg->record, g->record, newg->h.nobs * sizeof(Gauge_record));
  return newg;
}


static int obs_sort_compare_by_time(Gauge_record *r1, Gauge_record *r2)
   {
   /* Compare time values.  Return -1, 0, 1 for <, =, > comparison. */

   if (r1 == NULL) return 1;
   if (r2 == NULL) return -1;

   /* Compare year value */
   if (r1->time.year < r2->time.year) return -1;
   if (r1->time.year > r2->time.year) return 1;

   /* Compare jday value */
   if (r1->time.jday < r2->time.jday) return -1;
   if (r1->time.jday > r2->time.jday) return 1;

   /* Compare month value */
   if (r1->time.month < r2->time.month) return -1;
   if (r1->time.month > r2->time.month) return 1;

   /* Compare day value */
   if (r1->time.day < r2->time.day) return -1;
   if (r1->time.day > r2->time.day) return 1;

   /* Compare hour value */
   if (r1->time.hour < r2->time.hour) return -1;
   if (r1->time.hour > r2->time.hour) return 1;

   /* Compare minute value */
   if (r1->time.minute < r2->time.minute) return -1;
   if (r1->time.minute > r2->time.minute) return 1;
   
   /* Compare second value */
   if (r1->time.sec < r2->time.sec) return -1;
   if (r1->time.sec > r2->time.sec) return 1;

   return 0;
   }

/*************************************************************/
/*                                                           */
/*                find_network_in_gauge_complex              */
/*                                                           */
/*************************************************************/
Gauge_network *find_network_in_gauge_complex(Gauge_complex *gc, 
																						 char *netName)
{
	/* Finds a network of name 'netName' in the given gauge_complex.
	   Returns:
		   network, if found in the gauge_complex.
			 NULL, if not found.
	*/
	int j;
	
	for (j=0; j<gc->h.nnet; j++)
		if (strcmp(netName, gc->net[j]->h.name) == 0)
		  return(gc->net[j]);
	/* No such network name found. */
	return(NULL);
}

/*************************************************************/
/*                                                           */
/*                     find_gauge_radarSite                  */
/*                                                           */
/*************************************************************/
char *find_gauge_radarSite(char *netName)
{
	/* Given the name of a gauge network, find the radar site to which
		 the network belongs. This info is presently contained only in 
		 Fisher's database file 'radar.dat'.
  */
	int status=EOF;
	FILE *dbfile;
	char line[1000];
	static char radarSite_from_dbfile[6];
	char netName_from_dbfile[6];
	
	dbfile = fopen("/usr/local/trmm/GVBOX/data/sitelist/radar.dat", "r");
	if (dbfile == NULL)
	{
		fprintf(stderr, "Error opening database file: %s\n", "radar.dat");
		goto quit;
	}
	while (fgets(line, sizeof(line), dbfile) != NULL) {
	  /* The first 2 lines contain header info. Skip past comments. */
	  if (line[0] == '#') continue;
	  status = sscanf(line, "%*s %3s %4s %*s %*s", netName_from_dbfile,
					  radarSite_from_dbfile);
	  if (strcmp(netName_from_dbfile, netName) == 0) break;
	}
	while (status != EOF);

 quit:
	fclose(dbfile);
	if (status == EOF)
	{
		fprintf(stderr, "** Gauge network: %s not found in database file: %s\n",
						netName, "radar.dat");
	  strcpy(radarSite_from_dbfile, "???");
	}

	return(radarSite_from_dbfile);
}

/*************************************************************/
/*                                                           */
/*                 Gsort_gauge_by_time                       */
/*                                                           */
/*************************************************************/
Gauge *Gsort_gauge_by_time(Gauge *g)
{
  /* Allocates memory via Gcopy_gauge */
  Gauge *newg;
  newg = Gcopy_gauge(g);
  if (newg == NULL) return newg;

  qsort((void *)newg->record, newg->h.nobs, sizeof(Gauge_record),
		(int (*)(const void *, const void *))obs_sort_compare_by_time);

  return newg;
}
  
/*************************************************************/
/*                                                           */
/*                 Gsort_network_by_time                     */
/*                                                           */
/*************************************************************/
Gauge_measurement_at_time *Gsort_network_by_time(Gauge_network *gnet)
{
  /*
   * A. Sort each gauge by time.
   * B. Merge gauges by time.
   */
  Gauge_measurement_at_time *gmat;
  Gauge **sorted_gauges;
  Gauge *g;
  int *index, done;

  int i;

  if (gnet == NULL) return NULL;

  /* Sort each gauge. */
  sorted_gauges = (Gauge **)calloc(gnet->h.ngauge, sizeof(Gauge *));
  for (i=0; i<gnet->h.ngauge; i++) {
	/* Test. */
	g = gnet->gauge[i];
	g = Gsort_gauge_by_time(gnet->gauge[i]);  /* Allocates new Gauge. */
	sorted_gauges[i] = g;
  }

  /* Now merge the data by collecting all gauge measurements for
   * each time.
   *
   * a. Keep indexes, one for each gauge, and only bump
   *    the index when the time is 'oldest' and the data is
   *    inserted into the target.
   *
   */
  index = (int *)calloc(gnet->h.ngauge, sizeof(int));
  done = 0;
  while(! done) {
	/* Done when all indexes exceed the # of all gauge measurements. */
	for (i=0; i<gnet->h.ngauge; i++) {
	  if (index[i] < sorted_gauges[i]->h.nobs) {
		done = 0;
		break;
	  }
	  done = !0;
	}

  } /* End while */  
  free(index);
  return gmat;
}



