#ifndef IO_H_
#define IO_H_

#include <graph_defs.h>

int get_lines(FILE *f);
long *read_edgelist_from_file(FILE *f, long *nNodes, long m, char **rev);
int read_graph_from_edgelist(graph_t* G, long *EL, long n, long m);

#endif

