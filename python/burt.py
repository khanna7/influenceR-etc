#!/usr/bin/env python

import sys
from scipy.sparse import csr_matrix, vstack
import numpy as np

# Convert a DIMACS file to a Compressed-Sparse-Row Matrix
def csv_to_csr(gf, delim=',', st=0, skip=1):
    for i in range(skip):
        gf.next() # skip first line

    name_to_index = {}
    def add_name(name):
        try:
            return name_to_index[name]
        except:
            name_to_index[name] = len(name_to_index)
            return name_to_index[name]

    I = []
    J = []
    V = []

    for line in gf:
        tok = line.strip().split(delim)
        u = add_name(tok[st])
        v = add_name(tok[st+1])

        I.append(u)
        J.append(v)
        V.append(1.0)
    
        I.append(v)
        J.append(u)
        V.append(1.0)
    
    n = max(max(I), max(J)) + 1

    return name_to_index, csr_matrix((V, (I, J)), shape=(n, n))

    
    
# Find the eigenvector corresponding to the largest eigenvalue. Make sure it's positive.
def eff_net_size(A):
    S = A.dot(A.T) # index (i,j) = # of neighbors shared by i and j
    Q = A.multiply(S) # broadcast. (i,j) = # of shared neighbors if i and j are neighbors, 0 else
    qsum = Q.sum(0)
    deg = A.sum(0)
    ens = deg - (qsum/deg)
    arr = np.asarray(ens)[0]
    return arr

def constraint_all_(A):
    deg = np.asarray(A.sum(0))[0]
    n = len(deg)
    jq_s = [A.multiply(A[i]) for i in range(n)]
    jq = hstack(jq_s)
    
    degscale = csr_matrix(np.concatenate([deg] * n))
    jq_deg = jq.multiply(degscale)
    jq_deg.eliminate_zeros()
    
    jq_deg.data = 1 / jq_deg.data
    
    jq_deg_lil = jq_deg.tolil()
    cl = []
    for i in range(n):
        c = jq_deg_lil[i].reshape((n,n))
        cl.append(c.sum(1).getA1())
    S = np.asarray(cl)
    
    d = deg[i]
    C = (1/d)*S + (1/d)
    np.fill_diagonal(C, 0)
    C2 = np.multiply(C, C)
    
        
    return C2.sum(0)
    


def constraint_all_(A):
    
    n = A.shape[0]
    
    deg = A.sum(0).getA1()

    sjs = list()
    for i in range(n):
        jq = A.multiply(A[i]) # jq[j,k] = 1 iff (i,j) and (i, k) are both edges
        # row jq[j] contains 1's in all the q-spots for vertex j
    
        degscale = csr_matrix(deg)
        degscale[:,i] = 0
        jq_deg = jq.multiply(degscale) # qs[j][q] = deg(q) if q in N(j), else 0
        jq_deg.eliminate_zeros()
    
        jq_deg.data = 1 / jq_deg.data
        sjs.append(jq_deg.sum(1).getA1())
   
    S = np.asarray(sjs)
    dr = 1 / deg
    C = dr * S + dr
    np.fill_diagonal(C, 0)
    C2 = np.multiply(C, C)
    cs = C2.sum(0)
          
    return cs
    
def constraint_all(A):
    
    n = A.shape[0]
    
    
    deg = A.sum(0).getA1()
    degscale = csr_matrix(deg)
    Ascale = A.multiply(degscale)
    
    #np.fill_diagonal(Ascale, 0)

    #sjs = list()
    
    jqs = [A.multiply(Ascale[i]) for i in range(n)] # qs[j][q] = deg(q) if q in N(j), else 0
    
    jq = vstack(jqs)    
    jq.eliminate_zeros()
    jq.data = 1 / jq.data
    
    S = jq.sum(1).getA1()
    S.shape = n,n
    dr = 1 / deg
    C = dr * S + dr
    np.fill_diagonal(C, 0)
    C2 = np.multiply(C, C)
    cs = C2.sum(0)

    return cs
    

def constraint_i(A, i):
    #deg = np.asarray(A.sum(0))[0]
    deg = A.sum(0).getA1()
    jq = A.multiply(A[i]) # jq[j,k] = 1 iff (i,j) and (i, k) are both edges
    # row jq[j] contains 1's in all the q-spots for vertex j
    
    degscale = csr_matrix(deg)
    degscale[:,i] = 0
    jq_deg = jq.multiply(degscale) # qs[j][q] = deg(q) if q in N(j), else 0
    jq_deg.eliminate_zeros()
    
    jq_deg.data = 1 / jq_deg.data
    
    Sj = jq_deg.sum(1)
    Cj = (1/deg[i])*Sj + (1/deg[i])
    Cj[i] = 0 # don't care about this row
    Cj2 = np.multiply(Cj, Cj)
    
    return Cj2 
    
def constraint_i_old(A, i):
    #deg_ = A.sum(0)
    deg = np.asarray(A.sum(0))[0]
    jq = A.multiply(A[i]) # jq[j,k] = 1 iff (i,j) and (i, k) are both edges
    # row jq[j] contains 1's in all the q-spots for vertex j

    jq_deg = jq.multiply(deg)


    # from http://stackoverflow.com/questions/26248654/numpy-return-0-with-divide-by-zero
    with np.errstate(divide='ignore'):
        Ij = 1 / jq_deg
        Ij[jq_deg == 0] = 0
        
    Sj = Ij.sum(1)
    Cj = (1/deg[i])*Sj + (1/deg[i])
    Cj[i] = 0 # don't care about this row
    Cj2 = np.multiply(Cj, Cj)
    return Cj2 #np.sum(Cj2)
    
    
def constraint(A):
    n = A.shape[0]
    cs = []
    for i in range(n):
        c = np.sum(constraint_i(A, i))    
        cs.append(c)
    return cs

def main():
    
    if sys.argv[1] == "-ens":
        func = eff_net_size
    elif sys.argv[1] == "-constraint":
        func = constraint
    else:
        print "Error! Usage: python burt.py {-ens,-constraint} graph.dim"
        exit(1)
    
    f = open(sys.argv[2])
    names, A = csv_to_csr(f)
    x = func(A)
    for name,i in names.items():
        print name + "," + str(x[i])
        
if __name__ == "__main__":
    main()
