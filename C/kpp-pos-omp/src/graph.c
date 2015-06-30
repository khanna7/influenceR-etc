#include <graph_defs.h>
#include <graph_kernels.h>
#include <graph_gen.h>

#include "graph.h"

long BFS_parallel_frontier_expansion_with_distance(graph_t* G, long src, long diameter, double *distance);

/* read graph and return pointer. write number of vertices into *n. rewritten to have same interface as read_matrix_from_file */
graph_t* read_graph_from_file(char *f, int *n)
{
  graph_t *g = (graph_t *) malloc(sizeof(graph_t));
	graph_gen(g, f, "dimacs");
	*n = g->n;
  return g;
}

problem_t *make_problem_from_file(char *f, int *n)
{  

  graph_t *g = read_graph_from_file(f, n);
  
  problem_t *p = (problem_t *) malloc(sizeof(problem_t));
  p->graph = g;
  p->distance = NULL;
  p->round = 0;
  return p;
}

/* TODO: what's a good diameter value? */
double * BFS_multiple(graph_t *g, int *src, int k, double *res)
{
	int n = g->n;
	for (int i = 0; i < k*n; i++)
		res[i] = INFINITY;
	
	for (int i = 0; i < k; i++) {
		BFS_parallel_frontier_expansion_with_distance(g, (long) src[i], 75, &res[i*n]);
	}
	return res;
}

double * BFS_single(graph_t *g, int src, double *res)
{
	int n = g->n;
	for (int i = 0; i < n; i++)
		res[i] = INFINITY;
	BFS_parallel_frontier_expansion_with_distance(g, (long) src, 75, res);
	return res;
}
	
/* adadpted from breadth_first_search.c : BFS_parallel_frontier_expansion */
long BFS_parallel_frontier_expansion_with_distance(graph_t* G, long src, long diameter, double *distance) {

    attr_id_t* S;
    long *start;
    char* visited;
    long *pSCount;
#ifdef DIAGNOSTIC
    double elapsed_time;
#endif
#ifdef _OPENMP_KP
    omp_lock_t* vLock;
#endif

    long phase_num, numPhases;
    long count;


#ifdef _OPENMP_KP

OMP("omp parallel")
    {
#endif

        attr_id_t *pS, *pSt;
        long pCount, pS_size;
        long v, w;
        int tid, nthreads;
        long start_iter, end_iter;    
        long j, k, vert, n;
#ifdef _OPENMP_KP
        int myLock;
#endif

#ifdef _OPENMP_KP    
        long i;
        tid = omp_get_thread_num();
        nthreads = omp_get_num_threads();
#else
        tid = 0;
        nthreads = 1;
#endif


#ifdef DIAGNOSTIC    
        if (tid == 0)
            elapsed_time = get_seconds();
#endif

        if (tid == 0)  
            numPhases = diameter + 1;
        n = G->n;

        pS_size = n/nthreads + 1;
        pS = (attr_id_t *) malloc(pS_size*sizeof(attr_id_t));
        assert(pS != NULL);

        if (tid == 0) {  
            S = (attr_id_t *) malloc(n*sizeof(attr_id_t));
            visited = (char *) calloc(n, sizeof(char));
            start = (long *) calloc((numPhases+2), sizeof(long));
            pSCount = (long *) malloc((nthreads+1)*sizeof(long));
#ifdef _OPENMP_KP
            vLock = (omp_lock_t *) malloc(n*sizeof(omp_lock_t));
#endif
        }

#ifdef _OPENMP_KP    
OMP("omp barrier")
OMP("omp for")
        for (i=0; i<n; i++) {
            omp_init_lock(&vLock[i]);
        }
#endif

#ifdef _OPENMP_KP
OMP("omp barrier")
#endif

        if (tid == 0) {
            S[0] = src;
            visited[src] = (char) 1;
            count = 1;
            phase_num = 0;
            start[0] = 0;
            start[1] = 1;
			distance[src] = 0;
        }


#ifdef _OPENMP_KP
OMP("omp barrier")
#endif

        while (start[phase_num+1] - start[phase_num] > 0) {

            pCount = 0;

            start_iter = start[phase_num];
            end_iter = start[phase_num+1];
#ifdef _OPENMP_KP
OMP("omp for")
#endif
            for (vert=start_iter; vert<end_iter; vert++) {

                v = S[vert];
                for (j=G->numEdges[v]; j<G->numEdges[v+1]; j++) {
                    w = G->endV[j]; 
                    if (v == w)
                        continue;
#ifdef _OPENMP_KP
                    myLock = omp_test_lock(&vLock[w]);
                    if (myLock) {
#endif
                        if (visited[w] != (char) 1) { 
							distance[w] = distance[v] + 1;
                            visited[w] = (char) 1;
                            if (pCount == pS_size) {
                                /* Resize pS */
                                pSt = (attr_id_t *)
                                    malloc(2*pS_size*sizeof(attr_id_t));
                                memcpy(pSt, pS, pS_size*sizeof(attr_id_t));
                                free(pS);
                                pS = pSt;
                                pS_size = 2*pS_size;
                            }
                            pS[pCount++] = w;
                        }
#ifdef _OPENMP_KP
                        omp_unset_lock(&vLock[w]);
                    }
#endif
                }
            }


#ifdef _OPENMP_KP
OMP("omp barrier")
#endif            
            pSCount[tid+1] = pCount;

#ifdef _OPENMP_KP
OMP("omp barrier")
#endif            

            if (tid == 0) {
                pSCount[0] = start[phase_num+1];
                for(k=1; k<=nthreads; k++) {
                    pSCount[k] = pSCount[k-1] + pSCount[k];
                }
                start[phase_num+2] = pSCount[nthreads];
                count = pSCount[nthreads];
                phase_num++;
            }

#ifdef _OPENMP_KP
OMP("omp barrier")
#endif
            for (k = pSCount[tid]; k < pSCount[tid+1]; k++) {
                S[k] = pS[k-pSCount[tid]];
            } 


#ifdef _OPENMP_KP
OMP("omp barrier")
#endif
        } /* End of search */

#ifdef DIAGNOSTIC
        if (tid == 0) {
            fprintf(stderr, "Search from vertex %ld," 
                    " No. of vertices visited: %ld\n", src, count);
        }
#endif

        free(pS);
#ifdef _OPENMP_KP    
OMP("omp barrier")
OMP("omp for")
        for (i=0; i<n; i++) {
            omp_destroy_lock(&vLock[i]);
        }
OMP("omp barrier")
#endif

        if (tid == 0) {
            free(S);
            free(start);
            free(visited);
            free(pSCount);
#ifdef _OPENMP_KP
            free(vLock);
#endif

        }

#ifdef _OPENMP_KP    
    }
#endif

#ifdef DIAGNOSTIC    
    elapsed_time = get_seconds() - elapsed_time;
    fprintf(stderr, "Time taken for BFS: %lf seconds\n", elpased_time);
#endif
    return count;
}




