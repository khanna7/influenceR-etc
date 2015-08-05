#include "bridging.h"
#include <assert.h>
#include <stdio.h>

#include <mpi.h>
#include <io.h>

/* Bridging is described here:
 * http://www.ncbi.nlm.nih.gov/pmc/articles/PMC2889704/
 * "Bridging is calculated by systematically deleting links and calculating the resultant changes 
 * in network cohesion (measured as the inverse average path length). The average change for each node's 
 * links provides an individual level measure of bridging."
 * */


int main(int argc, char *argv[])
{  

  MPI_Init(NULL, NULL);
  
  // Get the number of processes
  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  int *read_edgelist_from_file(FILE *f, int *EL, char **rev, long m);
  int read_graph_from_edgelist(graph_t* G, int *EL, long n, long m);
  
  int n; /* number of nodes */
  graph_t *G = read_graph_from_file(argv[1], &n);
  int m = G->m; /* number of edges */

  
  
  //double *bridging(graph_t *G, int *edgelist, double *scores);
  
  MPI_Gather(buf, delta, MPI_DOUBLE, scores, delta, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  
  if (rank == 0) {
	for (int i = 0; i < n; i++)
	  printf("%g\n", scores[i]);
  }
	

#ifdef VERBOSE
  fprintf(stderr, "Rank %d goodbye\n", rank);
#endif
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  return 0;
}
