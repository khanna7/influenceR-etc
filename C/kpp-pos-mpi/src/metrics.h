#include <stdio.h>

#ifndef METRICS_H_
#define METRICS_H_

// also includes non mpi helper functions. 

double get_next_state(double *D, int n, int *s, int k, double p, int *ua, int *va);

double kpmetric(double *D, int n, int *s, int m, int *argmin);
double kpmetric_check(double *D, int n, int *s, int m, int *prevargmin, int u, int v);
void gen_starting_set(int n, int k, int *s);
/* go from { i | s_i is TRUE } to [i0, i1, i2,...] */
void make_t(int *t, int *s, int n);
void print_t(int *t, int n);
void print_s(int *s, int n);
double *read_matrix_from_file(char *f, int *nvertices);
void regen(int *gen, int *s, int *t, int n, int k);

#endif
