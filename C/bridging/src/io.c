#define _GNU_SOURCE
#include <stdio.h>
#include <search.h>
#include <stdlib.h>
#include <graph_defs.h>

long name_to_index(char *name, char **rev, int m) {
  static long hash_len = 0;
  
  ENTRY e, *ep;
  
  if (hash_len == 0) {
    hcreate(m); // overestimate
  }
  e.key = name;
  ep = hsearch(e, FIND);
  
  if (ep == NULL) {
    e.data = (void *) hash_len;
    ep = hsearch(e, ENTER);
    rev[hash_len] = name;
    if (ep == NULL) {
      fprintf(stderr, "Error inserting into hashtable!\n");
      exit(1);
    }
    hash_len++;
  }

  return (long) ep->data;
}

int get_lines(FILE *f) {

  int c, m = 0;
  while (EOF != (c=getchar()))
    if (c=='\n')
      ++m;

  rewind(f);

  return m;
}

/* Read a CSV and interpret it as an edgelist. The first two columns will be
 * interpreted as sources' and targets' names. The array of char*s at rev will
 * be a mapping between vertex IDs (integers) and vertex names. These strings
 * are allocated by the function and not freed. The edgelist will also be
 * allocated by the function and not freed. The values at *n and *m will be
 * updated to reflect the number of nodes and vertices in the file.
 */
int *read_edgelist_from_file(FILE *f, long *nNodes, long m, char **rev) {

  long n=0;

  int i = 0;
  int *EL = (int *) malloc(m * sizeof(int));

  while(!feof(f)) {
    
    /* get source */
    char *src = NULL;
    getdelim(&src, 0, ',', f);
    int u = name_to_index(src, rev, m);
    
    /* get target */
    char *tar = NULL;
    getdelim(&tar, 0, ',', f);
    int v = name_to_index(tar, rev, m);
    
    /* advance file pointer */
    char *trash = NULL;
    getline(&trash, 0, f);
    if (trash)
      free(trash);
    
    EL[i*2] = u;
    EL[i*2+1] = v;
    
    i++;
  }
  
  if (i != m) {
    fprintf(stderr, "Wrong number of lines in file!\n");
    exit(1);
  }
  *nNodes = n;
  //*nEdges = m;

  return EL;
}

int read_graph_from_edgelist(graph_t *G, int *EL, long n, long m) {

    long i;
    long count, offset;
    int int_wt=1, *int_weight; // maybe some graphs have weights.
    long u, v;
    attr_id_t *src;
    attr_id_t *dest;
    attr_id_t *degree;

    src = (attr_id_t *) malloc (m * sizeof(attr_id_t));
    dest = (attr_id_t *) malloc(m * sizeof(attr_id_t));
    degree = (attr_id_t *) malloc(n * sizeof(attr_id_t));
    for (int i = 0; i < n; i++) degree[i] = 0;

    assert(src != NULL);
    assert(dest != NULL);
    assert(degree != NULL);

    int_weight = (int *) malloc(m * sizeof(int));
    assert(int_weight != NULL);

    count = 0;

    for (int i = 0; i < m; i++) {
      u = EL[2*i];
      v = EL[2*i+1];
      
      if ((u <= 0) || (u > n) || (v <= 0) || (v > n)) {
          fprintf(stderr, "Error reading edge # %d (%d, %d) in the input file."
                  " Please check the input graph file.\n", count+1, u, v);
          return 1;
      }
      src[count] = u-1;
      dest[count] = v-1;
      degree[u-1]++;
      degree[v-1]++;
      int_weight[count] = int_wt;
      
      count++;
    }

    if (count != m) {
        fprintf(stderr, "Error! Number of edges specified in problem line (%ld)" 
                " does not match the total number of edges (%ld) in file."
                " Please check"
                " the graph input file.\n", m, count);
        return 1;
    }

    /*
       for (i=0; i<m; i++) {
       fprintf(stderr, "[%d %d] ", src[i], dest[i]);
       }
     */

    G->endV = (attr_id_t *) calloc(2*m, sizeof(attr_id_t));
    assert(G->endV != NULL);

    G->edge_id = (attr_id_t *) calloc(2*m, sizeof(attr_id_t));
    assert(G->edge_id != NULL);

    G->numEdges = (attr_id_t *) malloc((n+1)*sizeof(attr_id_t));
    assert(G->numEdges != NULL);

    G->undirected = 1;
    G->weight_type = 1;
    G->zero_indexed = 0;

    G->n = n;
    G->m = 2*m;

    G->int_weight_e = (int *) malloc(G->m * sizeof(int));       
    assert(G->int_weight_e != NULL);

    /* ToDo: parallelize this step */
    G->numEdges[0] = 0; 
    for (i=1; i<=G->n; i++) {
        G->numEdges[i] = G->numEdges[i-1] + degree[i-1];
    }

    for (i=0; i<count; i++) {
        u = src[i];
        v = dest[i];
        
        offset = degree[u]--;
        G->endV[G->numEdges[u]+offset-1] = v;
        G->int_weight_e[G->numEdges[u]+offset-1] = int_weight[i];
        G->edge_id[G->numEdges[u]+offset-1] = i;

        offset = degree[v]--;
        G->endV[G->numEdges[v]+offset-1] = u;
        G->int_weight_e[G->numEdges[v]+offset-1] = int_weight[i];
        G->edge_id[G->numEdges[v]+offset-1] = i;
    } 

    /*          
    for (i=0; i<G->n; i++) {
        for (j=G->numEdges[i]; j<G->numEdges[i+1]; j++) {
            fprintf(stderr, "<%ld %ld %d> ", i+1, G->endV[j]+1, 
            G->int_weight_e[j]);
        }
    }
    */


    //free(buf);
    free(degree);
    free(src);
    free(dest);
    
    return 0;
}
