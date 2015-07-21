#!/usr/bin/env Rscript

library(igraph)
library(Matrix)

constraint_naive <- function(g, i) {
    c <- 0
    ni <- neighbors(g, i)
    di <- degree(g, v=i)
    for (j in ni) {
        nj <- neighbors(g, j)
        Q <- intersection(ni, nj)
        s <- sum(vapply(Q, function(q) ((1/di)*(1/degree(g, v=q))), 0)) + 1/di
        c <- c + (s*s)
    }
    c
}

constraint_LA <- function(g, i) {
  A <- get.adjacency(g, sparse=T)
  deg <- degree(g)
  
  idx <- as.numeric(neighbors(g, i))
  
  jq <- drop0(t(A*A[,i]) * A[,i])
    
  jqd <- drop0(jq * deg)
  
  #jqd[jqd==0] <- Inf 
  #jqdr <- 1/jqd
  jqd@x <- (1/jqd@x) * (1/deg[i])

  w <- colSums(jqd)
  
  w[idx] <- w[idx] + (1/deg[i])
  
  x <- w * w
  
  sum(x)
}

mycon <- function(g, i) {
  A <- get.adjacency(g, sparse=T)
  deg <- degree(g)
  
  idx <- as.numeric(neighbors(g, i))
  jq <- A[,idx] * A[,i]
  jqd <- drop0(jq * deg)
  
  #jqd@x <- (1/jqd@x) * (1/deg[i])
  jqd[jqd==0] <- Inf
  jqd <- (1/jqd) * (1/degnorm)

  w <- colSums(jqd)  
  w <- w + (1/deg[i])
  x <- w * w
  
  sum(x)
}


mycon_vec <- function(g, i) {
  A <- get.adjacency(g, sparse=T)
  deg <- degree(g)
  n <- length(deg)
  
  idx <- unlist(apply(A==1, 1, which))
  
  vecnorm <- matrix(sapply(V(g), function(i) (rep(A[,i], deg[i]))), nrow=n)
  #jq <- A[,idx] * A[,i]
  jq <- A[,idx] * vecnorm
  
  jqd <- drop0(jq * deg)
  
  degnorm = unlist(sapply(V(g), function(i) rep(deg[i], deg[i])))
  jd <- jqd * degnorm
  jd@x <- (1/jd@x)

  w <- colSums(jd)  
  w <- w + (1/deg[i])
  x <- w * w
  
  sum(x)
}


constraint_LA_vec <- function(g) {
  constraint <- vector(mode="numeric", length=vcount(g))
  
  A <- get.adjacency(g, sparse=T)
  deg <- degree(g)
  n <- length(deg)
  
  JQL <- sapply(V(g), function(i) t(A*A[,i]) * A[,i])
  JQS <- do.call(cbind, JQL)
  JQD <- drop0(JQS * deg)   
  
  for (i in V(g)) {
  
    #jq <- drop0(t(A*A[,i]) * A[,i])
    
    #jqd <- drop0(jq * deg)
  
    jqd <- JQD[,((i-1)*n+1):(i*n)]
  
    #jqd[jqd==0] <- Inf 
    #jqdr <- 1/jqd
    jqd@x <- (1/jqd@x) * (1/deg[i])

    w <- colSums(jqd)
  
  
    idx <- as.numeric(neighbors(g, i))
    w[idx] <- w[idx] + (1/deg[i])
  
    x <- w * w
  
    constraint[i] <- sum(x)
  }
  
  constraint
}


constraint_naive_all <- function(g) {
  sapply(V(g), function(i) constraint_naive(g, i))
}

mycon_all <- function(g) {
  sapply(V(g), function(i) mycon(g, i))
}

# funcs <- c(igraph=igraph::constraint, mycon=mycon_all)
# graphs <- c("R0-undir.dim", "fb4-undir.dim", "fb3-undir.dim", "fb2-undir.dim",  "fb1-undir.dim")
#
# fst <- T
# for (fn in graphs) {
#   g <- influenceR::dimacs.to.graph(fn)
#   print(fn)
#   for (func in names(funcs)) {
#     if (fst || func!="naive") {
#       print(func)
#       print(system.time(funcs[[func]](g)))
#     }
#   }
#   print("")
#   fst <- F
# }

