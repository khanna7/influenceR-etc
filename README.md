# hiv-social-networks

## Intro

The CCHE has compiled large biobehavioral network datasets on men who have sex with men (MSM) in South Chicago and in Southern India. These datasets are being used to design a variety of HIV prevention approaches in these high-risk groups.

In one intervention, the goal is to efficiently disseminate prevention information on prexposure prophylaxis (PrEP) within these networks. The CCHE has detailed data on the Facebook networks of MSM in Chicago, and cell-phone networks of MSM in India. One primary goal is to the find a set of “significant” nodes within these networks who can most efficiently disseminate information about PrEP and other prevention messaging.

In order to support this work, RCC is coding up algorithms in the literature to find significant nodes in a social network. These nodes can be targeted for interventions as specified above. Many of these algorithms scale poorly to large datasets, so parallelization is needed to leverage RCC’s Midway infrastructure. In addition to algorithmic software development, sample visualizations have been developed in order to help researchers at CCHE evaluated the results of their research.

## Node signifcance algorithms

Algorithms have been implemented in C, R, or Python depending on the computational demands of the relevant method. We will
ultimately package all these methods into an R package for distribution on CRAN. The methods along with their current status
are detailed below.

**Betweenness**: A common centrality measure for networks. We used a C/OpenMP code called [SNAP](http://snap-graph.sourceforge.net/) to compute this measure. The R [igraph](http://igraph.org/r/) package also contains an efficient implementation.

**Bridging**: This measure uses a link-deletion method, where the "cohesiveness" of the network is measured before and after deleting each edge. A node's bridging score is the average of the cohesiveness of all its incident edges. This algorithm scales poorly to large datasets, so RCC consultants wrote a C/MPI version leveraging the SNAP library, which can be run on an arbitrary number of cores on Midway. See the "C/bridging" folder.

**Key Players**: This method defines a measure of the cohesiveness of the network based on the minimum distance from all nodes to a selected set S. Combinatorial optimization is used to select the set itself. RCC consultants wrote an extremely optimized version of the metric, and wrote C/MPI code to search the possible solution space in parallel using a stochastic gradient descent method. This code can be run on an arbitrary number of cores, although in practice 4-8 cores was found to be sufficient.

**Eigencentrality**: This measure uses spectral graph methods. An eigenvector decomposition of the graph's adjacency matrix is performed. Node *i*'s eigencentrality score corresponds to index *i* in the largest eigenvector. Both Python and R have wrappers around ARPACK sparse eigendecomposition functions (in the Scipy and igraph packages, respectively).

**Burt's Effective Network Size and Burt's Network Contraint**: These related graph metrics were developed by Ronald Burt at the Booth School of Business. Effective Network Size measures the degree to which a node’s neighborhood is smaller if it has redundant links; Network Constraint measures how much a given node constrains a different node by connecting it to other parts of the network. After some algebraic manipulation, both of these metrics reduce to simple linear algebra which was coded in Python, then R.