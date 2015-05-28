#!/usr/bin/env python

import sys
import scipy.sparse as sp
import scipy.sparse.linalg as sl
from scipy import real

# Convert a DIMACS file to a Compressed-Sparse-Row Matrix
def dimacs_to_csr(gf):
    tok = gf.next().strip().split(' ')
    n = int(tok[2])
    m = int(tok[3])

    #print "nodes: %d, edges %d" % (n, m)

    I = []
    J = []
    V = []

    for line in gf:
        tok = line.strip().split(' ')[1:]
        u, v, w = [int(x) for x in tok]
        u -= 1
        v -= 1

        I.append(u)
        J.append(v)
        V.append(1.0)
    
        I.append(v)
        J.append(u)
        V.append(1.0)
    
    return sp.csr_matrix((V, (I, J)), shape=(n, n))

# Find the eigenvector corresponding to the largest eigenvalue. Make sure it's positive.
def eig_centrality(A):
    w, v = sl.eigs(A, k=1)
    x = real(v[:,0]) # real part of first eigenvector
    if (sum(x<0) > sum(x>0)):
        x *= -1
    return x
    

def main():
    f = open(sys.argv[1])
    A = dimacs_to_csr(f)
    x = eig_centrality(A)
    for i in x:
        print i
        
if __name__ == "__main__":
    main()