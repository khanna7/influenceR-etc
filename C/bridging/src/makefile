CC=icc
CFLAGS= -std=c99  -O3 -g 
GRAPH_INCLUDE= -I/home/sdjacobs/local/include -I../src-snap
GRAPH_LIBRARY = -L/home/sdjacobs/local/lib -lsnap
GRAPH_CFLAGS = $(GRAPH_INCLUDE) $(GRAPH_LIBRARY)

	
main: main.c bridging.c
	$(CC) $(CFLAGS) $(GRAPH_CFLAGS) -o bridging  main.c bridging.c

	
main-mpi: main-mpi.c bridging.c
	$(CC) $(CFLAGS) $(GRAPH_CFLAGS) -o bridging main-mpi.c bridging.c



