#include <stdio.h>
#include "gsl.h"

void main (int argc, char **argv)
{
  Gauge_complex *gc;
  int i;

  if (argc < 2) {
	fprintf(stderr, "Usage: %s gminfile1 [gminfile2 ...]]\n", argv[0]);
	exit(-1);
  }

  gc = Gconstruct_gauge_complex(argc-1, &argv[1], RAINGAUGE);

  for (i=0; i<gc->h.nnet; i++) {
	printf("\n\n\n============= Network %d =============\n\n\n", i);
	Gprint_network(gc->net[i]);
  }

  exit(0);
}
