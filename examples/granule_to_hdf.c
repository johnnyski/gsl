#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gsl.h"

void main (int argc, char **argv)
{
  Gauge_complex *gc;
  char *outfile;
  int i;
  struct stat stat_buf;

  if (argc < 3) {
	fprintf(stderr, "Usage: %s gminfile1 [gminfile2 ...]] out.hdf\n", argv[0]);
	exit(-1);
  }

  /* Complain, if the output file exists. */
  outfile = argv[argc-1];
  
  if (stat(outfile, &stat_buf) >= 0) { /* I don't want a pre-existing
										* file.
										*/
	fprintf(stderr, "File %s exists.\n", outfile);
	exit(-1);
  }

  gc = Gconstruct_gauge_complex(argc-2, &argv[1], RAINGAUGE);

  Gauge_complex_to_hdf(gc, outfile);

  exit(0);
}
