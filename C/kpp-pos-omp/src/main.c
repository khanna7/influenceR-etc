#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <omp.h>

#include "graph.h"
#include "graph_metrics.h"
typedef problem_t* data_t;
#define NEXT_STATE get_next_state_graph
#define READ_DATA read_graph_from_file



void gen_starting_set(int n, int k, int *s);
void make_t(int *t, int *s, int n);
void print_t(int *t, int n);
void print_s(int *s, int n);
void regen(int *gen, int *s, int *t, int n, int k);   


/* single core version: compute btter solutions for T seconds */
void driver(graph_t *g, int n, int k, double p, double tol, long maxsec)
{
	
  int np, rank, new_rank = 0, stop;

  double *fits;
  int *allsets;
  time_t start;

  srand(time(NULL));
    
  problem_t problem;
  problem.graph = g;
  problem.distance = NULL;
  problem.round = 0;
  
  double fit;

	int s[n];
	gen_starting_set(n, k, s);

  start = time(0);
	do {
		int u, v;

		fit = get_next_state_graph(&problem, n, s, k, p, &u, &v, run);
		
		if (u >= 0)
			s[u] = 0;
		if (v >= 0)
			s[v] = 1;
	} while(difftime(time(0), start) < maxsec);
  
	return fit;	
}

/* While we're working:
	 * compute better solutions for T seconds
	 * send my fit back to the master process.
	 * get a number back. if it's my rank, broadcast my s array to everyone! 
*/
void driver_parallel(graph_t *g, int n, int k, double p, double tol, long sec, long maxsec)
{
	
  int np, rank, new_rank = 0, stop;

  double *fits;
  int *allsets;
  time_t start, fullstart;

#pragma omp parallel shared(fits, allsets, new_rank, g, np, stop) private(rank, start, fullstart)
  {
    
    np =  omp_get_num_threads();
    rank = omp_get_thread_num();
    
    srand(time(NULL) + rank);
    
    if (rank == 0) {
      allsets = (int *) malloc(n * np * sizeof(int));
      fits = (double *) malloc(np * sizeof(double));
    }
    
    problem_t problem;
    problem.graph = g;
    problem.distance = NULL;
    problem.round = 0;
    
#pragma omp barrier
    
  	int *s = &allsets[rank * n];
  	gen_starting_set(n, k, s);
	
  	start = time(0), fullstart = start;
  	double *fit = &fits[rank];
    *fit = 0;
    double oldfit = 0;
	
  	int run = 0;
	
  	do {
		
  		start = time(0);
		
  		do {
  			int u, v;
		
  			*fit = get_next_state_graph(&problem, n, s, k, p, &u, &v, run);
				
  			if (u >= 0)
  				s[u] = 0;
  			if (v >= 0)
  				s[v] = 1;
  		} while(difftime(time(0), start) < sec);
      
		
  		//printf("Run %d, rank %d, fit %g\n", run, rank, *fit);
		
  		new_rank = 0;
		
#pragma omp barrier
      
  		/* Master process: find best fit. */
  		if (rank == 0) {
  			double max = 0;
  			for (int i = 0; i < np; i++) {
  				printf("Run %d, rank %d, fit %g\n", run, i, fits[i]);
  				if (fits[i] > max) {
  					max = fits[i];
  					new_rank = i;
  				}
  			}
  			if (max - oldfit < tol || (difftime(time(0), fullstart) > maxsec)) {
  				stop = 1;
  			}
  			oldfit = max;
  		}

#pragma omp barrier
		
  		/* update s, or send it */
      if (rank != new_rank) {
        int *best_s = &allsets[n * new_rank];
        for (int i = 0; i < n; i++)
          s[i] = best_s[i];
      }		
		
  	  run++;
      
#pragma omp barrier
      
  	} while(!stop);
  }
}


int main(int argc, char** argv) {
    

	int n = 0;
	
	graph_t *D = read_graph_from_file(argv[1], &n);

	int k = atoi(argv[2]);
	
	double p = atof(argv[3]), 
		tol = atof(argv[4]);
		
	long sec = atoi(argv[5]),
	 maxsec = atoi(argv[6]);

#ifdef OPENMP
	driver_parallel(D, n, k, p, tol, sec, maxsec);
#else
  driver(D, n, k, p, tol, maxsec);
#endif

}


void gen_starting_set(int n, int k, int *s) {
    memset(s, 0, n * sizeof(int));
    for(int i = 0; i < k; i++) {
        int t = rand() % n;
        while(s[t] != 0)
            t = (t + 1) % n;
        s[t] = 1;
    }
}

/* go from { i | s_i is TRUE } to [i0, i1, i2,...] */
void make_t(int *t, int *s, int n) {
	int j = 0;
	for(int i = 0; i < n; i++) {
		if (s[i] == 1) {
			t[j] = i;
			j++;
		}
	}
}


void print_t(int *t, int n) {
    printf("Set: ");
    for(int i = 0; i < n - 1; i++)
        printf("%d, ", t[i]);
    printf("%d\n", t[n-1]);
}

void print_s(int *s, int n) {
    printf("Set: ");
    for(int i = 0; i < n; i++)
		if (s[i] == 1)
        	printf("%d ", i+1);
    printf("\n");
}


/* gen[i] = 0 if i in t, 1 if in s*/
void regen(int *gen, int *s, int *t, int n, int k)
{
                
	int si = 0, ti = 0;
	for (int i = 0; i < n; i++) {
		if(gen[i] == 1) {
			s[si] = i;
			si++;
		}
		else {
			t[ti] = i;  
			ti++;
		}
	}   
	if (si != k || ti != (n-k)) {
		printf("error \n");
		exit(1);
	}               
                        
	return;
}       




