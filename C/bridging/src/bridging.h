#ifndef BRIDGING_H_
#define BRIDGING_H_

#include <graph_defs.h>

graph_t *read_graph_from_file(char *f, int *n);
long BFS_parallel_frontier_expansion_bridging(graph_t* G, long src, long diameter, double *distance, long ignore_edge0, long ignore_edge1);
double closeness(graph_t *G, long ignore_edge0, long ignore_edge1);
double bridging_vertex_precomp(graph_t *G, long v, double cls, double *closeness);

#endif
