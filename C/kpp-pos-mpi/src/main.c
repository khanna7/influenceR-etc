#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mpi.h>

#ifdef GRAPH
#include "graph.h"
#include "graph_metrics.h"
typedef graph_t* data_t;
#define NEXT_STATE get_next_state_graph
#define READ_DATA read_graph_from_file
#else
#include "metrics.h"
typedef double* data_t;
#define NEXT_STATE get_next_state
#define READ_DATA read_matrix_from_file
#endif


void gen_starting_set(int n, int k, int *s);
void make_t(int *t, int *s, int n);
void print_t(int *t, int n);
void print_s(int *s, int n);
void regen(int *gen, int *s, int *t, int n, int k);   



/* While we're working:
	 * compute better solutions for T seconds
	 * send my fit back to the master process.
	 * get a number back. if it's my rank, broadcast my s array to everyone! 
*/
void driver(int rank, int np, data_t D, int n, int k, double p, double tol, long sec, long maxsec)
{
	srand(time(NULL) + rank);
	
	int s[n];
	gen_starting_set(n, k, s);
	
	time_t start = time(0), fullstart = start;
	double fit = 0, oldfit = 0;
	
	int run = 0, stop = 0;
	
	double *fits = NULL;
	if (rank == 0)
		fits = (double *) malloc(sizeof(double) * np);
	
	do {
		
		start = time(0);
		
		do {
			int u, v;
		
			fit = NEXT_STATE(D, n, s, k, p, &u, &v, run);
					
			if (u >= 0)
				s[u] = 0;
			if (v >= 0)
				s[v] = 1;
		} while(difftime(time(0), start) < sec);
		
		//printf("Run %d, rank %d, fit %g\n", run, rank, fit);
		
		MPI_Gather(&fit, 1, MPI_DOUBLE, fits, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		int new_rank = 0;
		
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
		
		MPI_Bcast(&new_rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
		/* update s, or send it... */
		MPI_Bcast(s, n, MPI_INT, new_rank, MPI_COMM_WORLD);
		
		MPI_Barrier(MPI_COMM_WORLD);
		
		if (rank == new_rank) {
			printf("Best fit: %f (rank %d)\n", fit, new_rank);
			print_s(s, n);
		}
		
		run++;
		
		MPI_Bcast(&stop, 1, MPI_INT, 0, MPI_COMM_WORLD);
	} while(!stop);
}


int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int n = 0;
	
	data_t D = READ_DATA(argv[1], &n);

	int k = atoi(argv[2]);
	
	double p = atof(argv[3]), 
		tol = atof(argv[4]);
		
	long sec = atoi(argv[5]),
	 maxsec = atoi(argv[6]);

	driver(world_rank, world_size, D, n, k, p, tol, sec, maxsec);

    // Finalize the MPI environment.
    MPI_Finalize();
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




