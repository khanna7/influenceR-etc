#include "graph.h"
#include "graph_metrics.h"
#include "../src/metrics.h"

#define X(i, j, n) ((i) * (n) + (j))

/* D is a matrix where D[si,j] is the distance from s[si] to j. */
double kpmetric_graph(graph_t *g, double *D, int n, int *s, int *t, int k, int *argmin)
{
	double sum = 0;
	
	if (argmin != NULL)
		for (int i = 0; i < n; i++)
			argmin[i] = -1;
	
	for(int ti = 0; ti < n-k; ti++) {
		int i = t[ti];
		
		double min = INFINITY;
		
		for (int si = 0; si < k; si++) {
			double m = D[X(si, i, n)];
			if (m < min) {
				if (argmin != NULL)
					argmin[i] = si;
				min = m;
			}
		}
		
		if (min != 0 && min < INFINITY)
			sum += 1.0/min;
	}
	
	return sum/((double)n);
}
double kpmetric_graph_check(graph_t *g, double *D, int n, int *s, int *t, int k, int *prevargmin, int u, int v)
{
	
	double distance_v_to_all[n];
	BFS_single(g, v, distance_v_to_all);
	
	double sum = 0;
	
	for(int ti = 0; ti < n-k; ti++) {
		int i = t[ti];
		if (i == v)
			i = u;
		
		double min = INFINITY;
		
        int si = prevargmin[i];
        if (si != -1 && s[si] != u) {
            min = D[X(si, i, n)];
            double m = distance_v_to_all[i];
            if (m < min)
                min = m;
        }
        else {
            for (int si = 0; si < k; si++) {
                double m;
                if (s[si] != u)
                    m = D[X(si, i, n)];
                else
                    m = distance_v_to_all[i];
                if (m < min) {
                    min = m;
                }
            }
        }
		
		if (min != 0 && min < INFINITY)
			sum += 1.0/min;
	}
	
	return sum/((double)n);
}


/* Compute Metric 15 in Borghatti (19) */
/* D is a n-by-n matrix */
/* s is a list of indices, kp-set */
/* compute: X = sum(min(v not in s)(distance to w, w in s)) */
/* sum(1/X) / n */
double kpmetric_st(double *D, int n, int *s, int *t, int k, int *argmin)
{
	double sum = 0;

	if (argmin != NULL)
		for (int i = 0; i < n; i++)
			argmin[i] = -1;
	
    for(int ti = 0; ti < n-k; ti++) {
		
		int i = t[ti];
		
		double min = INFINITY;

		for (int si = 0; si < k; si++) {
			int j = s[si];
			double m = D[X(i, j, n)];
			if (m < min) {
				if (argmin != NULL)
					argmin[i] = j;
				min = m;
			}
		}
		
		if (min != 0 && min < INFINITY) /* This node is in s */
			sum += 1.0/((double)min);
    }
	
	return sum/((double) n);
}


/* Get next state function, probabilistically */
/* Method: pick a u randomly, and a v randomly. If fit improves, go for it. Otherwise, go for it with probability p. */
/* t = which(s) */
double get_next_state_graph_old(graph_t *graph, int n, int *gen, int k, double p, int *ua, int *va)
{
	int argmin[n];
	int s[k], t[n-k];
	regen(gen, s, t, n, k);
	
	double *distance = (double *) malloc(n*k*sizeof(double));
	//printf("start BFS-multiple %d\n", time(0));
	BFS_multiple(graph, s, k, distance); /* we can do fewer calls to this guy with some refactoring. */
	//printf("end")

	double fit = kpmetric_graph(graph, distance, n, s, t, k, argmin);
	int tries = 0;
	while (1) {
		int u = s[rand() % k];
		int v = t[rand() % (n-k)];
				
		double fit_ = kpmetric_graph_check(graph, distance, n, s, t, k, argmin, u, v);
		
		if (fit_ > fit || ((double) rand()/ ((double) RAND_MAX)) < p) {
			*ua = u;
			*va = v;
			free(distance);
			return fit_;
		}
		if (tries > 10000) {
			*ua = -1;
			*va = -1;
			free(distance);
			return fit;
		}
		
		tries++;
	}
}

double get_next_state_graph(problem_t *this, int n, int *gen, int k, double p, int *ua, int *va, int round)
{
	int argmin[n];
	int s[k], t[n-k];
	regen(gen, s, t, n, k);
	
  graph_t *graph = this->graph;
  
  if (round != this->round) {
    free(this->distance);
    this->distance = NULL;
    this->round = round;
  }

	if (this->distance == NULL) {
		this->distance = (double *) malloc(n*k*sizeof(double));
		if (!this->distance) {
			printf("error with malloc!");
			exit(1);
		}
		BFS_multiple(graph, s, k, this->distance);
	}

	double fit = kpmetric_graph(graph, this->distance, n, s, t, k, argmin);
	int tries = 0, u, v;
	while (1) {
		u = s[rand() % k];
#ifdef DEBUG
        // we want to match how we get a random v in the non-graph code. this sucks.
		v = rand() % n;
		while(gen[v] != 0)
			v = (v + 1) % n;
#else
        v = t[rand() % (n-k)];
#endif				

		
  double fit_ = kpmetric_graph_check(graph, this->distance, n, s, t, k, argmin, u, v);
		
	if (fit_ > fit || ((double) rand()/ ((double) RAND_MAX)) < p) {
		*ua = u;
		*va = v;
		fit = fit_;
		break;
	}
	if (tries > 10000) {
		*ua = -1;
		*va = -1;
		break;
	}
	
	tries++;
}
	
	/* replace row u with row v */
	double * new_distance = (double *) malloc(n*k*sizeof(double));
	if (! new_distance) {
		printf("error with malloc!");
		exit(1);
	}
	int j = 0; // index into new_distance
	int vdone = 0;
	for (int i = 0; i < k; i++) {
		if (!vdone && s[i] > v) {
			BFS_single(graph, v, &new_distance[j*n]);
			j++;
			vdone = 1;
		}
		if (s[i] == u)
			continue;
		
		// copy
		for (int a = 0; a < n; a++)
			new_distance[X(j, a, n)] = this->distance[X(i, a, n)];
		j++;
	}
	if (j != k) {
		BFS_single(graph, v, &new_distance[j*n]);
		j++;
	}
	if (j != k) {
		printf("fatal error!\n");
		exit(1);
	}
		
	double *tmp = this->distance;
	this->distance = new_distance;
	free(tmp);
	
	return fit;
}
