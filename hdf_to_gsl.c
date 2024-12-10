/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Gauge Software Library.
    Copyright (C) 1996  Mike Kolander, John Merritt of
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

	Moves all raingauge data from one 2A-56 HDF file into one GSL
	'Gauge_complex' structure.

	Similiarly, moves disdrometer data from one 2A-57 HDF file.

	A GSL 'Gauge_complex' structure contains data from all raingauges
	from all raingauge networks at one radar site.


  -----------------------------------------------------------------
	 Libraries required for execution of this code :
      -ltsdistk                    : tsdis toolkit
      -lmfhdf -ldf -ljpeg -lz      : HDF
      -lm                          : C math

  -----------------------------------------------------------------
		mike.kolander@trmm.gsfc.nasa.gov
		(301) 286-1540
*******************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBTSDISTK

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "gsl.h"
/* HDF 4.0r2 and TSDIS TOOLKIT 4.* */
#include "IO.h"
#include "IO_GV.h"
#include "TKerrHandle.h"

Gauge *build_disdrogauge(L2A_57_DISDROMETER *l2a57);
Gauge *build_raingauge(L2A_56_RAINGAUGE *l2a56);
Gauge_network *build_gauge_network(IO_HANDLE *ioh, NETDESC *netDesc,
																	 int productType);
Gauge_complex *Ghdf_to_gauge_complex(char *hdffile);

/***********************************************************************/
/* The following 4 functions are temporary until TSDIS incorporates
	 them into the toolkit. */

int TKreadRaingauge(IO_HANDLE *ioh, L2A_56_RAINGAUGE *l2astr)
{
	/********* TEST PURPOSES ONLY *************/
	l2astr->gaugeDesc.networkID = 0;
	strcpy(l2astr->gaugeDesc.gaugeName, "** name here **");
	strcpy(l2astr->gaugeDesc.gaugeType, "TIP");
	l2astr->hour[49] = 7;
	return(TK_SUCCESS);
}

int TKreadDisdrometer(IO_HANDLE *ioh, L2A_57_DISDROMETER *l2astr)
{
	/********* TEST PURPOSES ONLY *************/	
	l2astr->disdroDesc.networkID = 0;
	strcpy(l2astr->disdroDesc.disdroName, "** name here **");
	l2astr->hour[1] = 7;
	return(TK_SUCCESS);
}


int TKreadGauge(IO_HANDLE *ioh, void *l2astr)
{
	/********* TEST PURPOSES ONLY *************/
	if (ioh->productID == TK_L2A_56) TKreadRaingauge(ioh, l2astr);
	else  TKreadDisdrometer(ioh, l2astr);
	return(TK_SUCCESS);
}
#if 0
int TKreadNetHeader(IO_HANDLE *ioh, int *nnet, NETDESC **nDesc)
{
	/********* TEST PURPOSES ONLY *************/
	NETDESC *netDesc;
	int j;
	
	*nnet = 2;
	netDesc = (NETDESC *)calloc(*nnet, sizeof(NETDESC));
	for (j=0; j<2; j++)
	{
		strcpy(netDesc[j].networkName, "net??");
		netDesc[j].nValidSensor = 1;
		netDesc[j].networkID = j;
	}
	*nDesc = netDesc;
	return(TK_SUCCESS);
}
/***********************************************************************/
#endif


/***********************************************************/
/*                                                         */
/*                     build_disdrogauge                   */
/*                                                         */
/***********************************************************/
Gauge *build_disdrogauge(L2A_57_DISDROMETER *l2a57)
{
	Gauge *g;
	int j, k, nobs;

	/* Find out how many of the 1440 slots of the L2A_57_DISDROMETER
		 structure are non-empty.
		 Do this by searching from the end of the array for a non-zero time
		 value. */
	for (nobs=1440; nobs>0; nobs--)
		if ( (l2a57->hour[nobs-1]) || (l2a57->minute[nobs-1])) break;
	
	/* Gauge contains 'nobs' observations, each consisting of 20 bins. */
	g = (Gauge *)Gnew_gauge(nobs, 20);
	g->h.number = l2a57->disdroDesc.disdroNumber;
	g->h.name = (char *) strdup(l2a57->disdroDesc.disdroName);
	g->h.type = (char *) strdup("DSD");
	g->h.network = (char *) strdup("nnn");     /* ??? */
	g->h.resolution = l2a57->disdroDesc.resolution;
	g->h.lat = l2a57->disdroDesc.disdroLat;
	g->h.lon = l2a57->disdroDesc.disdroLong;
	g->h.azimuth = l2a57->disdroDesc.azimuth;
	g->h.range = l2a57->disdroDesc.range;
	g->h.elevation = l2a57->disdroDesc.elevation;
	g->h.nobs = nobs;
	g->h.nbin = 20;
	for (j=0; j<g->h.nobs; j++)
	{
		g->record[j].time.hour = l2a57->hour[j];
		g->record[j].time.minute = l2a57->minute[j];
		for (k=0; k<g->h.nbin; k++)
		  g->record[j].value[k] = l2a57->nConcentration[j][k];
	}
	fprintf(stderr, "DisdroGauge: %s contains %d recorded observations.\n",
					g->h.name, g->h.nobs);
	return(g);
}

/***********************************************************/
/*                                                         */
/*                     build_raingauge                     */
/*                                                         */
/***********************************************************/
Gauge *build_raingauge(L2A_56_RAINGAUGE *l2a56)
{
	Gauge *g;
	int j, nobs;
	
	/* Find out how many of the 1440 slots of the L2A_56_RAINGAUGE
		 structure are non-empty.
		 Do this by searching from the end of the array for a non-zero time
		 value. */
	for (nobs=1440; nobs>0; nobs--)
		if ( (l2a56->hour[nobs-1]) || (l2a56->minute[nobs-1])) break;
	
	/* Gauge contains 'nobs' observations, each consisting of 1 bin. */
	g = (Gauge *)Gnew_gauge(nobs, 1);
	g->h.number = l2a56->gaugeDesc.gaugeNumber;
	g->h.name = (char *) strdup(l2a56->gaugeDesc.gaugeName);
	g->h.type = (char *) strdup(l2a56->gaugeDesc.gaugeType);
	g->h.network = (char *) strdup("nnn");     /* ??? */
	g->h.resolution = l2a56->gaugeDesc.resolution;
	g->h.lat = l2a56->gaugeDesc.gaugeLat;
	g->h.lon = l2a56->gaugeDesc.gaugeLong;
	g->h.azimuth = l2a56->gaugeDesc.azimuth;
	g->h.range = l2a56->gaugeDesc.range;
	g->h.elevation = l2a56->gaugeDesc.elevation;
	g->h.nobs = nobs;
	g->h.nbin = 1;
	for (j=0; j<g->h.nobs; j++)
	{
		g->record[j].time.hour = l2a56->hour[j];
		g->record[j].time.minute = l2a56->minute[j];
		g->record[j].value[0] = l2a56->meanRainRate[j];
	}
	fprintf(stderr, "Raingauge: %s contains %d recorded observations.\n",
					g->h.name, g->h.nobs);
	return(g);
}

/***********************************************************/
/*                                                         */
/*                   build_gauge_network                   */
/*                                                         */
/***********************************************************/
Gauge_network *build_gauge_network(IO_HANDLE *ioh, NETDESC *netDesc,
																	 int productType)
{
	Gauge_network *gnet;
	L2A_57_DISDROMETER  l2a57;
  L2A_56_RAINGAUGE    l2a56;
	int j, status;
	
	gnet = Gnew_gauge_network(MAX_NETWORK_GAUGES);
	if (gnet == NULL) return(NULL);

	/* Read each gauge of one network from the HDF file into
		 toolkit structure 'l2a56' or 'l2a56', then move into a GSL
		 'Gauge' structure. */
	if (productType == TK_L2A_56)
	  for (j=0; j<netDesc->nValidSensor; j++)
		{
/******************************************************/
			memset(&l2a56, 0, sizeof(L2A_56_RAINGAUGE));
/******************************************************/
			status = TKreadGauge(ioh, &l2a56);
			if (status != TK_SUCCESS) TKreportError(status);
			gnet->gauge[j] = (Gauge *)build_raingauge(&l2a56);
		}
	else  /* productType == TK_L2A_57 */
	  for (j=0; j<netDesc->nValidSensor; j++)
		{
/******************************************************/
			memset(&l2a57, 0, sizeof(L2A_57_DISDROMETER));
/******************************************************/
			status = TKreadGauge(ioh, &l2a57);
			if (status != TK_SUCCESS) TKreportError(status);
			gnet->gauge[j] = (Gauge *)build_disdrogauge(&l2a57);
		}

	/* Fill the Gauge_network header values. */
	gnet->h.name = (char *) strdup(netDesc->networkName);
	gnet->h.type = (char *) strdup(gnet->gauge[0]->h.type);
	gnet->h.ngauge = netDesc->nValidSensor;
	return(gnet);
}

/***********************************************************/
/*                                                         */
/*                    Ghdf_to_gauge_complex                */
/*                                                         */
/***********************************************************/
Gauge_complex *Ghdf_to_gauge_complex(char *hdffile)
{
  /* Return gnet upon success.
   * Return NULL upon failure.
	 */
  IO_HANDLE   ioh;
  NETDESC   *netDesc; /* Pointer to an array of NETDESC structures.*/
  Gauge_complex *gcomplex;
  char *fileName;
  char radarSite[6], productString[6];
  int j, nnet=0, productType, status;

  if (hdffile == NULL) return(NULL);
	/* Bypass the leading (optional) directory pathname contained in the 
		 string 'hdffile'. Use 'fileName' to point to the actual file name. */
	fileName = strrchr(hdffile, '/');  /* Find leading '/', if exists. */
	if (fileName != NULL) fileName++;  /* Point 1 char past leading '/'. */
	else fileName = hdffile;           /* No leading '/'. */
	/* Get the productType and radarSite out of the filename. */
	status = sscanf(fileName, "%4s.%*6s.%4s", productString, radarSite);
	if (status == EOF) TKreportError(status);

	if (strncmp(productString, "2A56", 4) == 0)  /* Raingauge */
	  productType = TK_L2A_56;
	else if (strncmp(productString, "2A57", 4) == 0)  /* Disdrometer */
	  productType = TK_L2A_57;
	else
	{
		fprintf(stderr, "Unexpected HDF product type in filename: %s.\n",
						fileName);
		return(NULL);
  }
	status = TKopen(hdffile, productType, TK_READ_ONLY, &ioh);
	if (status != TK_SUCCESS) TKreportError(status);
	/* Read an array of NETDESC structures from the HDF file. */
	if (nnet == 0)
	  fprintf(stderr, "Warning: hdf_to_gsl.c: nnet = %d\n", nnet);
	status = TKreadNetHeader(&ioh, nnet, &netDesc);
	if (status != TK_SUCCESS) TKreportError(status);

	/* Create a Gauge_complex structure, and fill its header values. */
	gcomplex = (Gauge_complex *)Gnew_gauge_complex(MAX_GAUGE_NETWORKS);
	gcomplex->h.radarSite = (char *) strdup(radarSite);
	gcomplex->h.nnet = nnet;
	/* Read each gauge_network from the HDF file into a GSL 'Gauge_network'
	   structure. */
	for (j=0; j<nnet; j++)
	{
	  gcomplex->net[j] = build_gauge_network(&ioh, &netDesc[j], productType);
		if (gcomplex->net[j] == NULL) return(NULL);
	}

	TKclose(&ioh);
	return(gcomplex);
}
#endif
