#ifndef MYGRAPH_H_
#define MYGRAPH_H_

#include <graph_defs.h>

typedef struct problem_struct {
  graph_t *graph;
  int round;
  double *distance;
} problem_t;

graph_t *read_graph_from_file(char *f, int *n);
problem_t *make_problem_from_file(char *f, int *n);
double *BFS_multiple(graph_t *g, int *src, int k, double *res);
double *BFS_single(graph_t *g, int src, double *res);

#endif
