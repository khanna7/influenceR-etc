#ifndef GRAPH_METRIC_H
#define GRAPH_METRIC_H

#include <graph_defs.h>

double kpmetric_graph(graph_t *g, double *D, int n, int *s, int *t, int k, int *argmin);
double kpmetric_st(double *D, int n, int *s, int *t, int k, int *argmin);

double kpmetric_graph_check(graph_t *g, double *D, int n, int *s, int *t, int k, int *prevargmin, int u, int v);

double get_next_state_graph(problem_t *this, int n, int *gen, int k, double p, int *ua, int *va, int round);

#endif
