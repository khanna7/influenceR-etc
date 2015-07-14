library(igraph)

constraint_naive <- function(g, i) {
    c <- 0
    ni <- neighbors(g, i)
    di <- degree(g, v=i)
    for (j in ni) {
        nj <- neighbors(g, j)
        Q <- intersection(ni, nj)
        s <- sum(sapply(Q, function(q) ((1/di)*(1/degree(g, v=q))))) + 1/di
        c <- c + (s*s)
    }
    c
}

mycon <- function(g, i) {
  A <- get.adjacency(g, sparse=F)
  deg <- degree(g)
  
  jq <- t(A*A[,i]) * A[,i]
  
  jqd <- jq * deg
  
  jqd[jqd==0] <- Inf
  
  jqdr <- 1/jqd
  
  v <- jqdr * (1/deg[i])
  
  w <- colSums(v)
  
  idx <- neighbors(g, i)
  w[idx] <- w[idx] + (1/deg[i])
  
  x <- w * w
  
  sum(x)
}
