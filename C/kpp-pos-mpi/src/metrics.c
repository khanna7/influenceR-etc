#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "metrics.h"

#define X(i, j, n) ((i) * (n) + (j))
#define DUMB_INF 2147483647

/* Get next state function, probabilistically */
/* Method: pick a u randomly, and a v randomly. If fit improves, go for it. Otherwise, go for it with probability p. */
/* t = which(s) */
double get_next_state(double *D, int n, int *s, int k, double p, int *ua, int *va)
{
	int argmin[n];

	int t[k];
	make_t(t, s, n);
	

	double fit = kpmetric(D, n, t, k, argmin);
	int tries = 0;
	while (1) {
		int u = t[rand() % k];
		int v = rand() % n;
		while(s[v] != 0)
			v = (v + 1) % n;
				
		double fit_ = kpmetric_check(D, n, t, k, argmin, u, v);
		
		if (fit_ > fit || ((double) rand()/ ((double) RAND_MAX)) < p) {
			*ua = u;
			*va = v;
			return fit_;
		}
		if (tries > 10000) {
			*ua = -1;
			*va = -1;
			return fit;
		}
		
		tries++;
	}
}

/* Compute Metric 15 in Borghatti (19) */
/* D is a n-by-n matrix */
/* s is a list of indices, kp-set */
/* compute: X = sum(min(v not in s)(distance to w, w in s)) */
/* sum(1/X) / n */
double kpmetric(double *D, int n, int *s, int k, int *argmin)
{
	double sum = 0;
    for(int i = 0; i < n; i++) {
		
		double min = INFINITY;
		if (argmin != NULL)
        	argmin[i] = -1;

		for (int j = 0; j < k; j++) {
			double m = D[X(i, s[j], n)];
			if (m < min) {
				if (argmin != NULL)
					argmin[i] = s[j];
				min = m;
			}
		}
		
		if (min != 0 && min < INFINITY) /* This node is in s */
			sum += 1.0/min;
    }
	
	return sum/n;
}


/* Take out u, add v */
double kpmetric_check(double *D, int n, int *s, int k, int *prevargmin, int u, int v)
{
	double sum = 0;
    for(int i = 0; i < n; i++) {
		
		double min = INFINITY;
		int pj = prevargmin[i];

        if (pj == u) {
            for (int j = 0; j < k; j++) {
                if (s[j] == u)
                    continue;
                double m = D[X(i, s[j], n)];
                if (m < min) {
                    min = m;
                }
            }
        }
        else if (pj != -1)
            min = D[X(i, pj, n)];
		
		double m = D[X(i, v, n)];
		if (m < min)
			min = m;
		
		if (min != 0 && min < INFINITY) /* This node is in s */
			sum += 1.0/min;	
    }
	
	return sum/((double) n);
}


double *read_matrix_from_file(char *s, int *nvertices)
{
	FILE *f = fopen(s, "r");
	int n;
    fscanf(f, "%d\n", &n);
	double *D = (double *) malloc(n * n * sizeof(double));
    
    int v;
	for (int i = 0; i < n*n; i++) {
        fscanf(f, " %d", &v);
        D[i] = (v != DUMB_INF) ? (double) v : INFINITY;
    }
	*nvertices = n;
	return D;
}
