#include <assert.h>
#include <stdio.h>

#include "io.h"
#include "bridging.h"

#ifdef USE_MPI
#include <mpi.h>
#endif

/* Bridging is described here:
 * http://www.ncbi.nlm.nih.gov/pmc/articles/PMC2889704/
 * "Bridging is calculated by systematically deleting links and calculating the resultant changes 
 * in network cohesion (measured as the inverse average path length). The average change for each node's 
 * links provides an individual level measure of bridging."
 * */


int main(int argc, char *argv[])
{  

#ifdef USE_MPI
  MPI_Init(NULL, NULL);
  
  // Get the number of processes
  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif  

  FILE *f = fopen(argv[1], "r");

  long n, m = get_lines(f);
  char **names = (char **) malloc(m * sizeof(char *));
  int *EL = read_edgelist_from_file(f, &n, m, names);
  printf("%d\n", n);
  graph_t *G = malloc(sizeof(graph_t));
  read_graph_from_edgelist(G, EL, n, m);
  m = G->m;
  
  double *scores = (double *) malloc(n * sizeof(double));
#ifdef USE_MPI
  if (size > 1)
      bridging_MPI(G, EL, scores);
  else
#endif
    bridging(G, EL, scores);
 
#ifdef USE_MPI
  if (rank == 0) {
#endif
	for (int i = 0; i < n; i++)
	  printf("%g\n", scores[i]);
#ifdef USE_MPI
   }
#endif

#ifdef VERBOSE
  fprintf(stderr, "Rank %d goodbye\n", rank);
#endif
 
#ifdef USE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  return 0;
}
