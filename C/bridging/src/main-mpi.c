#include "bridging.h"
#include <assert.h>
#include <stdio.h>

#include <mpi.h>

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
  
 	int n; /* number of nodes */
  graph_t *G = read_graph_from_file(argv[1], &n);
	int m = G->m; /* number of edges */

  
	/* 1) compute closeness by edge in file */
	
  int bufsize = ceil(((double)m) / size), delta = bufsize/2;
  int start = rank * delta, end = start + delta;
  end = end > m ? m : end;
#ifdef VERBOSE
  fprintf(stderr, "Rank %d range: %d-%d\n", rank, start, end); 
#endif 
  double *buf = malloc(bufsize * sizeof(double));
  int *edgeidx = malloc(bufsize * sizeof(int));
  
  FILE *f = fopen(argv[1], "r");
  
  /* skip line */
  int n_, m_;
  fscanf(f, "p sp %d %d\n", &n_, &m_);
  assert(n_ == G->n);
  assert(m_*2 == G->m);
  
  int nlines = 0, i=0, u, v;
  long j, k;
  
  while(!feof(f)) {
#ifdef VERBOSE
    if (nlines == start) {
      fprintf(stderr, "Rank %d found start\n", rank); 
    }
#endif
    
    u = -2, v = -2;
    fscanf(f, "e %d %d 1\n", &u, &v);
    assert(u!=-2 && v!=-2);
    u--, v--;
    
    if (nlines >= start && nlines < end) {
      
      /* Find edge numbers */
      for (j=G->numEdges[u]; v != G->endV[j] && j<G->numEdges[u+1]; j++);
      for (k=G->numEdges[v]; u != G->endV[k] && k<G->numEdges[v+1]; k++);
      assert(j != G->numEdges[u+1]);
      assert(k != G->numEdges[v+1]);
      
      /* Calculate closeness */
      buf[i] = closeness(G, j, k);
      edgeidx[i] = j;
      buf[i+1] = buf[i];
      edgeidx[i+1] = k;
      i+=2;

#ifdef VERBOSE
      fprintf(stderr, "CBE %d %d %g\n", j, k, buf[i]);
#endif
      
    }
    nlines++;
  
#ifdef VERBOSE 
    if (nlines == end) {
      fprintf(stderr, "Rank %d done reading edges\n", rank);
    }
#endif
  }
  
  
  
  double *closeness_buf = NULL;
  int *edge_indices = NULL;
  if (rank == 0) {
    closeness_buf = malloc(bufsize*size * sizeof(double));
    edge_indices = malloc(bufsize*size * sizeof(int));
  }
  MPI_Barrier(MPI_COMM_WORLD);
  
  MPI_Gather(buf, bufsize, MPI_DOUBLE, closeness_buf, bufsize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Gather(edgeidx, bufsize, MPI_INT, edge_indices, bufsize, MPI_INT, 0, MPI_COMM_WORLD);
  
  
  double *closeness_by_edge = malloc(m * sizeof(double));
  /* Fill REAL closeness_by_edge matrix */
    
  if (rank == 0) {
    for (int i = 0; i < m; i++) {
  	  closeness_by_edge[edge_indices[i]] = closeness_buf[i];
#ifdef VERBOSE
      printf("CBE %d %g\n", edge_indices[i], closeness_buf[i]); 
#endif
    }
    
    free(closeness_buf);
    free(edge_indices);
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(closeness_by_edge, m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  free(buf);
  free(edgeidx);
	
  //fprintf(stderr, "Rank %d: total processed %d\n",rank, i);
	

	/* 2) compute bridging score by NODE. Parallization here may be more trouble than it's worth. But we already have the resources. */
  delta = ceil(((double)n) / size);
  start = rank * delta, end = start + delta;
  end = end > n ? n : end;
  
	double cls = closeness(G, -1, -1); // normal closeness (use all edges)

  buf = (double *) malloc(delta * sizeof(double));

	for (int v = start; v < end; v++) 
		buf[v-start] = bridging_vertex_precomp(G, v, cls, closeness_by_edge);
	
  
	double *scores = NULL;
  if (rank == 0) {
    scores = malloc(delta * size * sizeof(double));
    assert(scores != NULL);
  }
  
  MPI_Gather(buf, delta, MPI_DOUBLE, scores, delta, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  
	if (rank == 0) {
		for (int i = 0; i < n; i++)
			printf("%g\n", scores[i]);
	}
	
	/* Freeing the memory causes a strange error that I cannot track down. oh well. */

#ifdef VERBOSE
  fprintf(stderr, "Rank %d goodbye\n", rank);
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  
  return 0;
}
