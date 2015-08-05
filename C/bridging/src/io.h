#ifndef IO_H_
#define IO_H_

int *read_edgelist_from_file(FILE *f, int *EL, char **rev, long m);
int read_graph_from_edgelist(graph_t* G, int *EL, long n, long m);

#endif

