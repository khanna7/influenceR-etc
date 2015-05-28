#include "bridging.h"

#include <mpi.h>

/* Bridging is described here:
 * http://www.ncbi.nlm.nih.gov/pmc/articles/PMC2889704/
 * "Bridging is calculated by systematically deleting links and calculating the resultant changes 
 * in network cohesion (measured as the inverse average path length). The average change for each node's 
 * links provides an individual level measure of bridging."
 * */




int main(int argc, char *argv[])
{  
	// Initialize the MPI environment
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

 	int n; /* number of nodes */
    graph_t *G = read_graph_from_file(argv[1], &n);
	int m = (G->m)/2; /* number of edges */
	
	/* 1) compute closeness by edge ish */
	
	int delta = ceil(((double)m) / size);
	int start = rank * delta, end = start + delta;
	end = end > m ? m : end;

	double *buf = calloc(delta, sizeof(double));
    
    fprintf(stderr, "Rank %d: range is %d to %d\n", rank, start, end);	
	/* iterate to start of edge block */
	long edges = 0;
	
	int i = 0;
	for (long v = 0; v < n; v++) {
		for (long j=G->numEdges[v]; j<G->numEdges[v+1]; j++) {
			long w = G->endV[j];
			if (v < w) {
				if (edges >= start) {
					buf[i] = closeness(G, j);
					i++;
				}
				edges++;
				if (edges == end)
					goto done;
			}	
		}
	}
	
done:

  fprintf(stderr, "Rank %d: total processed %d\n",rank, i);
    	
  /* allgather to get the closeness array */
  	
  double *closeness_by_edge_half;
  if (rank == 0) 
    closeness_by_edge_half = calloc(m, sizeof(double));
  MPI_Allgather(buf, delta, MPI_DOUBLE, closeness_by_edge_half, delta, MPI_DOUBLE, MPI_COMM_WORLD);
  
  /* Everyone is doing the following, even though we could just do it once. */
  double *closeness_by_edge = malloc(G->m * sizeof(double));
  
  i = 0;
  for (long v = 0; v < n; v++) {
    for (long j=G->numEdges[v]; j<G->numEdges[v+1]; j++) {
      long w = G->endV[j];
      if (v < w) {
        closeness_by_edge[j] = closeness_by_edge_half[i];
        i++;
      }
      else {
        for (long k=G->numEdges[w]; k<G->numEdges[w+1]; k++) {
          if (v == G->endV[k]) {
            if (k > j) {
              printf("fatal error\n");
              exit(1);
            }
            closeness_by_edge[j] = closeness_by_edge_half[j];
          }
        }
      }
    }       
  }   
	
	/* 2) compute bridging score by NODE. Parallization here may be more trouble than it's worth. But we already have the resources. */
	
	delta = ceil(((double) n) / size);
	start = rank * delta;
	end = start + delta;
	end = end > n ? n : end;
	
	free(buf);
	buf = calloc(delta, sizeof(double));
	
	double cls = closeness(G, -1); // normal closeness (use all edges)

	for (int v = start; v < end; v++) 
		buf[v - start] = bridging_vertex_precomp(G, v, cls, closeness_by_edge);
	
	double *scores = NULL;
	if(rank == 0) 
		scores = calloc(n, sizeof(double));
	MPI_Gather(buf, delta, MPI_DOUBLE, scores, delta, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	if (rank == 0) {
		for (int i = 0; i < n; i++)
			printf("%d %g\n", i, scores[i]);
	}
	
	/* Freeing the memory causes a strange error that I cannot track down. oh well. */
	
 	MPI_Finalize();
    return 0;
}
