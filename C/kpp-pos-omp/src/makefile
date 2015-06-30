CFLAGS= -std=c99 -Wall -g -fopenmp
GRAPH_INCLUDE= -I/home/sdjacobs/local/include
GRAPH_LIBRARY = -L/home/sdjacobs/local/lib -lsnap
GRAPH_CFLAGS = -DGRAPH $(GRAPH_INCLUDE) $(GRAPH_LIBRARY)
TARGETS = main.c

keyplayer: metrics.c main.c
	$(CC) $(CFLAGS) -o keyplayer  $(TARGETS) metrics.c
	
keyplayer-graph: graph_metrics.c main.c
	$(CC) $(CFLAGS) $(GRAPH_CFLAGS) -o keyplayer-graph  $(TARGETS) graph_metrics.c graph.c

graphmetric:
	$(CC) $(CFLAGS) $(INCLUDE) $(SNAP_LIBRARY) -openmp -o test-gm test-graphmetric.c $(TARGETS)

clean:
	rm -f keyplayer keyplayer-graph

