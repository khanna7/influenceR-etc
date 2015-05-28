CC=mpicc
CFLAGS= -std=c99 -Wall -g
GRAPH_INCLUDE= -I/project/jschnei1/software/include -I../src-snap
GRAPH_LIBRARY = -L/project/jschnei1/software/lib -lsnap
GRAPH_CFLAGS = -openmp -DGRAPH $(GRAPH_INCLUDE) $(GRAPH_LIBRARY)
TARGETS = main.c

keyplayer: metrics.c main.c
	$(CC) $(CFLAGS) -o keyplayer  $(TARGETS) metrics.c
	
keyplayer-graph: graph_metrics.c main.c
	$(CC) $(CFLAGS) $(GRAPH_CFLAGS) -o keyplayer-graph  $(TARGETS) graph_metrics.c graph.c

graphmetric:
	$(CC) $(CFLAGS) $(INCLUDE) $(SNAP_LIBRARY) -openmp -o test-gm test-graphmetric.c $(TARGETS)

clean:
	rm -f keyplayer keyplayer-graph

