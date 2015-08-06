#include <graph_defs.h>

#ifndef BRIDGING_H
#define BRIDGING_H

double *bridging(graph_t *G, long *edgelist, double *scores);
double *bridging_MPI(graph_t *G, long *edgelist, double *scores);

#endif
