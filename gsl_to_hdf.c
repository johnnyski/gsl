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

	Moves one granule (24 hrs) of raingauge or disdrometer data
	from one GSL 'Gauge_complex' structure into one 2A-56 or 2A-57 HDF file.

	Checks granule integrity; ie, checks that all gauge observations
	in the Gauge_complex structure are from the same 24 hr period.

	In general, a GSL 'Gauge_complex' structure contains any amount of data
	from any number of raingauges from any number of raingauge networks
	located at one radar site.


  -----------------------------------------------------------------
	 Libraries required for execution of this code :
      -ltsdistk                    : tsdis toolkit
      -lmfhdf -ldf -ljpeg -lz      : HDF

  -----------------------------------------------------------------
		mike.kolander@trmm.gsfc.nasa.gov
		(301) 286-1540
*******************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBTSDISTK


#include <stdio.h>
#include <string.h>
#include "gsl.h"

/* HDF 4.0r2 and TSDIS TOOLKIT 4.* */
#include "IO.h"
#include "IO_GV.h"


int fill_l2a56(L2A_56_RAINGAUGE *l2a56, Gauge *gauge, int networkID);
int fill_l2a57(L2A_57_DISDROMETER *l2a57, Gauge *gauge, int networkID);
NETDESC *build_netDesc(Gauge_complex *gcomplex);


/***********************************************************/
/*                                                         */
/*                        fill_l2a56                       */
/*                                                         */
/***********************************************************/
int fill_l2a56(L2A_56_RAINGAUGE *l2a56, Gauge *gauge, int networkID)
{

	/* Checks that all gauge observations are from the same 24 hr period.
		 Returns: OK, if success.
		          ABORT, if fail.
  */
	int j, n, this_jday;
	
	if (gauge == NULL) return(ABORT);
	/* Zero out the L2A_56_RAINGAUGE structure before we move
	   any data into it... Many Gauges are only partly full. */
	memset(l2a56, 0, sizeof(L2A_56_RAINGAUGE));
			
	l2a56->gaugeDesc.networkID = networkID;
	n = strlen(gauge->h.type);
	if (n > 21) n = 21;
	strncpy(l2a56->gaugeDesc.gaugeType, gauge->h.type, n);
	n = strlen(gauge->h.name);
	if (n > 21) n = 21;
	strncpy(l2a56->gaugeDesc.gaugeName, gauge->h.name, n);
	n = 21;
	memset(l2a56->gaugeDesc.gaugeMakeModel, 0, n); /* N/A */
	l2a56->gaugeDesc.gaugeNumber  = gauge->h.number;
	l2a56->gaugeDesc.gaugeLat     = gauge->h.lat;
	l2a56->gaugeDesc.gaugeLong    = gauge->h.lon;
	l2a56->gaugeDesc.range        = gauge->h.range;
	l2a56->gaugeDesc.azimuth      = gauge->h.azimuth;
	l2a56->gaugeDesc.xCoordinate  = 0;
	l2a56->gaugeDesc.yCoordinate  = 0;
	l2a56->gaugeDesc.resolution   = gauge->h.resolution;
	l2a56->gaugeDesc.elevation    = gauge->h.elevation;
	  
	/* The number of observations cannot exceed 1440 (24hr*60min/hr). */
	if (gauge->h.nobs > 1440)
	{
		fprintf(stderr, "gauge->h.nobs = %d ... exceeds HDF limit of 1440.\n",
						gauge->h.nobs);
		return(ABORT);
	} 

	this_jday = gauge->record[0].time.jday;
	for (j=0; j<gauge->h.nobs; j++)
	{
		if (gauge->record[j].time.jday == this_jday)
		{
			l2a56->hour[j]               = gauge->record[j].time.hour;
			l2a56->minute[j]             = gauge->record[j].time.minute;
			/* Don't touch l2a56->meanRainRate_scale[j] */
			/*		  l2a56->meanRainRate[j]       = gauge->record[j].rate; */
			l2a56->meanRainRate[j]       = gauge->record[j].value[0];
		}
		else  /* Observations not all from same day */
		{
			fprintf(stderr, "Raingauge observations not all from same day.\n");
			return(ABORT);
		}
	}  /* end for (j=0; j<gauge->h.nobs; j++) */

	return(OK);
}

/***********************************************************/
/*                                                         */
/*                        fill_l2a57                       */
/*                                                         */
/***********************************************************/
int fill_l2a57(L2A_57_DISDROMETER *l2a57, Gauge *gauge, int networkID)
{
	int j, k, n, this_jday;

	if (gauge == NULL) return(ABORT);
	/* Zero out the L2A_57_DISDROMETER structure before we move
	   any data into it... Many Gauges are only partly full. */
	memset(l2a57, 0, sizeof(L2A_57_DISDROMETER));

	l2a57->disdroDesc.networkID = networkID;
	n = strlen(gauge->h.name);
	if (n > 21) n = 21;
	strncpy(l2a57->disdroDesc.disdroName, gauge->h.name, n);
	n = 21;
	memset(l2a57->disdroDesc.disdroMakeModel, 0, n); /* N/A */
	l2a57->disdroDesc.disdroNumber  = gauge->h.number;
	l2a57->disdroDesc.disdroLat     = gauge->h.lat;
	l2a57->disdroDesc.disdroLong    = gauge->h.lon;
	l2a57->disdroDesc.range        = gauge->h.range;
	l2a57->disdroDesc.azimuth      = gauge->h.azimuth;
	l2a57->disdroDesc.resolution   = gauge->h.resolution;
	l2a57->disdroDesc.elevation    = gauge->h.elevation;
	  
	/* The number of observations cannot exceed 1440 (24hr*60min/hr). */
	if (gauge->h.nobs > 1440)
	{
		fprintf(stderr, "gauge->h.nobs = %d ... exceeds HDF limit of 1440.\n",
						gauge->h.nobs);
		return(ABORT);
	}

	this_jday = gauge->record[0].time.jday;
	for (j=0; j<gauge->h.nobs; j++)
	{
		if (gauge->record[j].time.jday == this_jday)
		{
			l2a57->hour[j]               = gauge->record[j].time.hour;
			l2a57->minute[j]             = gauge->record[j].time.minute;
			for (k=0; k<gauge->h.nbin; k++)
			  l2a57->nConcentration[j][k] = (int16)gauge->record[j].value[k];
		}
		else  /* Observations not all from same day. */
		{
			fprintf(stderr, "Disdrometer observations not all from same day.\n");
			return(ABORT);
		}
	}  /* end for (j=0; j<gauge->h.nobs; j++) */

	return(OK);
}

/***********************************************************/
/*                                                         */
/*                       build_netDesc                     */
/*                                                         */
/***********************************************************/
NETDESC *build_netDesc(Gauge_complex *gcomplex)
{
	/* Create and fill an array of toolkit 'NETDESC' structures,
		 one per network.
	*/
	int j, n;
	NETDESC *netDesc;
	
	netDesc = (NETDESC *)calloc(gcomplex->h.nnet, sizeof(NETDESC));
	if (netDesc == NULL) return(NULL);
	for (j=0; j<gcomplex->h.nnet; j++)
	{
		n = strlen(gcomplex->net[j]->h.name);
		if (n > 21) n = 21;
		strncpy(netDesc[j].networkName, gcomplex->net[j]->h.name, n);
		netDesc[j].nValidSensor = gcomplex->net[j]->h.ngauge;
		netDesc[j].networkID = j;
	}

	return(netDesc);
}

/***********************************************************/
/*                                                         */
/*                    Gauge_complex_to_hdf                 */
/*                                                         */
/***********************************************************/
int Gauge_complex_to_hdf(Gauge_complex *gcomplex, char *hdffile)
{
	/*
		 Moves one granule (24 hrs) of raingauge or disdrometer data
		 from one GSL 'Gauge_complex' structure into one HDF file.

		 Checks granule integrity; ie, checks that all gauge observations
		 in the Gauge_complex structure are from the same 24 hr period.

		 Returns: OK, if success.
		          ABORT, if failure.
	*/
  IO_HANDLE ioh;
	NETDESC   *netDesc; /* Pointer to an array of NETDESC structures.*/
	L2A_57_DISDROMETER l2a57;
  L2A_56_RAINGAUGE   l2a56;
	int i, j, productType, status;
	
  if (gcomplex->net[0] == NULL) return(ABORT);
  if (hdffile == NULL) return(ABORT);

  if (strcmp(gcomplex->net[0]->h.type, "DSD") == 0)
	  productType = TK_L2A_57;  /* Disdrometer */
	else
	  productType = TK_L2A_56;  /* Raingauge */

	status = TKopen(hdffile, productType, TK_NEW_FILE, &ioh);
	if (status != TK_SUCCESS)
	{
		fprintf(stderr, "Gauge_complex_to_hdf(): Error opening hdffile.\n");
		return(ABORT);
	}
	
	/* Move header info from each Gauge_network into a toolkit
		 'NETDESC' structure. Note 'build_netDesc()' returns an array
		 of filled NETDESC structures. */
	netDesc = (NETDESC *)build_netDesc(gcomplex);
	if (netDesc == NULL)
	{
		fprintf(stderr, "Gauge_complex_to_hdf(): Error creating 'netDesc' structure\n");
		goto quit;
	}
	/* Write the array of NETDESC structures into the HDF file. */
	status = TKwriteNetHeader(&ioh, gcomplex->h.nnet, netDesc);
	if (status != TK_SUCCESS) goto quit;

	/* Write each gauge_network from the GSL structure into the HDF file.*/
	for (j=0; j<gcomplex->h.nnet; j++)
	{
		/* Write each gauge of one network into a toolkit 'l2a56' or
			 'l2a57' structure, then into the HDF file. */
		if (productType == TK_L2A_56)  /* Raingauge */
			for (i=0; i<gcomplex->net[j]->h.ngauge; i++)
			{ 
				status = fill_l2a56(&l2a56, gcomplex->net[j]->gauge[i], j);
				if (status != OK) break;
				status = TKwriteGauge(&ioh, &l2a56);
				if (status != TK_SUCCESS) break;
			}		
		else  /* Disdrometer */
			for (i=0; i<gcomplex->net[j]->h.ngauge; i++)
			{ 
				status = fill_l2a57(&l2a57, gcomplex->net[j]->gauge[i], j);
				if (status != OK) break;
				status = TKwriteGauge(&ioh, &l2a57);
				if (status != TK_SUCCESS) break;
			}
	} /* end for (j=0; j<gcomplex->nnet; j++) */

 quit:
	TKclose(&ioh);
	if (status == TK_SUCCESS) return(OK);
	else return(ABORT);
}
#endif
