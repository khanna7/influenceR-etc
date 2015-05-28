#!/usr/bin/env Rscript

library(igraph)
library(Matrix)

# igraph package has arpack functions
# evcent - eigenvector centrality
# betweenness - vertex betweenness

dimacs.to.graph <- function(fname) {
    x <- read.table(fname, skip=1)
    el <- as.matrix(x[2:3])
    
    graph.edgelist(el, directed=F)
}

eigencentrality <- function(g) {
  evcent(g, scale=F)$vector
}

# 1/2 the values of our betweenness code, which is because this is UNDIRECTED for real
btwnness <- function(g) {
  betweenness(g, directed=F) 
}

ens <- function(g) {
  A <- get.adjacency(g)   # This will be sparse, which is great.
  S <- crossprod(A)       # S[i,j] = # of shared neighbors between i,j
  Q <- A * S              # Q[i,j] = # of shared neighbors if i and j are neighbors, 0 else
  qsum <- rowSums(Q)
  deg <- rowSums(A)
  ens <- deg - (qsum/deg)
}

main = function(args) {
  func <- args[[1]]
  graph <- dimacs.to.graph(args[[2]])
  
  #load('pruned-fb-networks.RData')
  #network <- as.network(.GlobalEnv[[args[[2]]]])
  
  
  centrality <- c(eigen=eigencentrality, betweenness=btwnness, ens=ens)
  
  x <- centrality[[func]](graph)
  
  write.table(x, quote=F, row.names=F, col.names=F)
}

if (! interactive()) {
  args <- commandArgs(trailingOnly = TRUE)
  main(args)
}