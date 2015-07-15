#!/usr/bin/env Rscript

library(igraph)
library(Matrix)
library(Rcpp)

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

constraint <- function(g) {
  
  A <- get.adjacency(g, sparse=T)
  n <- dim(A)[1]
  deg <- rowSums(A)

  constraint_i <- function(i) {
    
    #jqd <- (A * A[i,]) * deg
    #jqd[,i] <- 0
    #jqd@x <- 1/jqd@x  # In place reciprocal. Alternative: jqd[jqd==0] <- Inf; jqd <- 1/jqd
    
    # process sparse does this: jq <- drop0(t(A*A[,i]) * A[,i]); jqd <- drop0(jq * deg)
    jqd <- process_sparse(A, A[i,], deg)
    
    #jqd <- drop0(jq * deg)
    jqd <- drop0(jqd)
    jqd@x <- (1/jqd@x) * (1/deg[i])
         
    Sj <- colSums(jqd)
  
    idx <- as.numeric(neighbors(g, i))
    Sj[idx] <- Sj[idx] + (1/deg[i])
    
    Sj2 <- Sj * Sj
    sum(Sj2)
  }
  
  sapply(1:n, constraint_i)
}

cppFunction('NumericVector c_process_sparse(IntegerVector I, IntegerVector J, NumericVector X, NumericVector Ai, NumericVector deg) {
  int n = X.size();
  NumericVector out(n);

  for(int p = 0; p < n; p++) {
    int j = J[p];
    int i = I[p];
    out[p] = ((X[p] * Ai[j]) * Ai[i]) * deg[j];
    //out[p] = X[p] * Ai[j] * Ai[i] * deg[j];
    //out[p] = (out[p] == 0 ? 0 : 1/out[p]);
  }
  
  return out;
}')

process_sparse <- function(A, Ai, deg) {
  M <- as(A, 'TsparseMatrix')
  x <- c_process_sparse(M@i, M@j, M@x, Ai, deg)
  M@x <- x
  M
}

main <- function(args) {
  
  graph <- dimacs.to.graph(args[[2]])
  func <- args[[1]]
  centrality <- c(eigen=eigencentrality, betweenness=btwnness, ens=ens, constraint=constraint)
  
  x <- centrality[[func]](graph)
  
  write.table(x, quote=F, row.names=F, col.names=F)
}

if (! interactive()) {
  args <- commandArgs(trailingOnly = TRUE)
  main(args)
}
