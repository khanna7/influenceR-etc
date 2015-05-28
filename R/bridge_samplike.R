

library(statnet)
library(foreign)
library(sna)
library(igraph)

detach(package:igraph)

setwd("c:/misc/diffnet/") 
# input Stata file
data2 <- read.dta("samplike.dta")
data2$ado   <- NULL
data2$study <- NULL

bridges_toprint <- c(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

bridge_net_toprint <- c(0, 0, 0)
Digits=3
commun <- 1
for (commun in 1:1) {

  data <- data2[which (data2$commun==commun), ]
  data$commun   <- NULL
  noms <- data
  dyad <-reshape(noms, direction="long",  varying = 2:5, sep="",
                idvar = "data$id", timevar="alter")
  dyad  <- dyad[!is.na(dyad$nom),]  
  dyad$alter <- NULL
  ex_net <- as.network(dyad)                        #convert to network
  
  N <- network.size(ex_net)

  distance <- geodist(ex_net, inf.replace=(N-1), count.paths=TRUE) #calculate distance matrix
  DIST <- distance$gdist
  CLOS  = 1/(DIST+1e-07)                                           #convert to closeness
  diag(CLOS) <- 0
  ave_clos = ((sum(sum(CLOS))) / (N^2 - N))                        
  
  # bridging
  change_mat <- matrix(data=0, nrow=N, ncol=N)
  KDIST      <- matrix(data=0, nrow=N, ncol=N)
  changes    <- c(0, 0, 0, 0, 0)                          # initialize 5 vars: i j original_link new_link change
  ex_mat <- as.sociomatrix(ex_net)
  j<-1
  for (j in 1:N) {
    k<-1
    for (k in 1:N) {
    if (j == k) {next}
      ex_mat2 <- ex_mat
      ex_mat2[j, k] <- abs(ex_mat2[j,k]-1)
      new_distance <- geodist(as.network(ex_mat2), inf.replace=(N-1), count.paths=TRUE)
      DIST2 <- new_distance$gdist
      CLOS2  = 1/(DIST2+1e-07)
    diag(CLOS2) <- 0
    ave_clos2 = ((sum(sum(CLOS2))) / (N^2 - N))

    #Can change which is first original or new one
    Clos_Diff = (ave_clos - ave_clos2)
    changes2 <- cbind(j, k, ex_mat[j,k], ex_mat2[j,k], Clos_Diff)
    changes  <- rbind(changes, changes2)
    change_mat[j,k] = Clos_Diff
    
    #K local bridge: Change in distance between 2 nodes when the link between them is removed or added
    KDIST[j,k] <- DIST[j,k] - DIST2[j,k]
  }
} 

#Calculate bridging scores average link deletion and addition
# Link changes in 2 separate matrices
del_mat = (change_mat * ex_mat)
add_mat = (change_mat * abs(ex_mat-1))

# Get Nodal Scores
out_deg <- (degree(ex_net, cmode="outdegree"))  
in_deg <- (degree(ex_net, cmode="indegree"))  

del_ave_out <- (rowSums(del_mat) / out_deg*2)         #calculate average change scores 
del_ave_in  <- (rowSums(del_mat) / in_deg*2)          #it is *2 because we sum over all i!=j
add_ave_out <- (rowSums(add_mat) / (N-1-out_deg)*2)   #calculate average change scores 
add_ave_in  <- (rowSums(add_mat) / (N-1-in_deg)*2)    #by number of non-links for each case

# Get Normalized score #
# First identify all non-pendants
not_ex_mat  <- (ex_mat - (ex_mat * t(ex_mat)))  # unreciprocated links
ns_nt     <- rowSums(not_ex_mat)
nr_nt     <- colSums(not_ex_mat)
not_pend  <- as.integer((ns_nt+nr_nt+ rowSums(ex_mat)) >=2)

#calculate bridging for star network then divide it into raw scores
B_star       <- ((.5*N)/(N-1))
del_norm1    <- ((del_ave_out / B_star) *100)
del_norm_out <- (del_norm1 * not_pend)  
del_norm2    <- ((del_ave_in / B_star) *100)
del_norm_in  <- (del_norm2 * not_pend)  


add_norm = ((add_ave_out) * (2*(N-1)))
del_ave <- (((del_ave_out) + (del_ave_in))/2)
#Calculate network level bridging
bistar=0.5/(N-1)
  del_ave_nbi = del_ave/bistar
  del_net <- sum(del_ave_nbi)/N
  del_net2 <- sum(del_ave_nbi *not_pend)/ sum(not_pend)
  
add_net <- sum(N*(add_ave_out))

#Calculate K-local bridging 
KDIST <- KDIST * ex_mat # keep only distance changes for existing links
kbr_out <- rowSums(KDIST)
kbr_in  <- colSums(KDIST)

#Betweenness Bridging
library(igraph)
exnet_graph <- graph.adjacency(as.sociomatrix(ex_net))
eb   <- edge.betweenness(exnet_graph)
# eb_scores <- as.numeric(cbind((get.edgelist(exnet_graph)), eb))
# tkplot(exnet_graph)
detach(package:igraph)

 # Creates a matrix and fills the cells with the edge betweenness scores
 eb_mat <- matrix(0, N, N)
 counter<-1
 o<-1
  p<-1
   for (o in 1:N) {
    for (p in 1:N) {
    # if there is a link for that dyad we put in it value from the eb vector then increment to next value
     if ((ex_mat[o,p])==1) {(eb_mat[o,p] <- eb[counter]) & (counter<- (counter+1))} 
    }
  }
 eb_out <- (rowSums(eb_mat)/out_deg)
 eb_in  <- (colSums(eb_mat)/in_deg)

 # Or just calculate node betweenness divide by 2 then: 2*between+n-1/outdeg
 between <- betweenness(ex_net) 
 between <- between/2                                # Need to divide by 2
 eb_between <- ((2*between)+(N-1)) / out_deg

 ids <- seq(1, N, 1)
 bridges <- abs(cbind(commun, ids, del_ave_out, del_norm_out, del_ave_in, del_norm_in, eb_out, eb_in, eb_between, kbr_out, kbr_in))
 #bridge_net <- cbind(commun, del_net, del_net2, del_net3, del_net4, add_net)
 bridge_net <- cbind(commun, del_net, del_net2)
  
  bridges_toprint<- rbind(bridges_toprint, bridges)
  bridge_net_toprint <- rbind(bridge_net_toprint, bridge_net)
  plot(ex_net, displaylabels=TRUE, displayisolates = TRUE, label.pos=5, label.cex=.8,
       vertex.cex=2, vertex.col=7)
  title(main=paste("Network", commun))
}

bridges_toprint <- round(bridges_toprint, digits=3)
bridge_net_toprint <- round(bridge_net_toprint, digits=3)
bridges_toprint <- bridges_toprint[2:(NROW(bridges_toprint)), ]
bridge_net_toprint <- bridge_net_toprint[2:(NROW(bridge_net_toprint)), ]
write.table(bridges_toprint, file="bridges_sl.txt", append=T, quote=FALSE, sep=" ", row.names=F, col.names=FALSE)
write.table(bridge_net_toprint, file="bridges_net_sl.txt", append=T, quote=FALSE, sep=" ", row.names=F, col.names=FALSE)

bridges.df <- data.frame(bridges_toprint)
cor(bridges.df)


