#!/usr/bin/python

import sys

# Convert a csv file to dimacs, dict
def csv_in(gf, dim_f, dict_f, delim=',', st=0, skip=1):
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

    for line in gf:
        tok = line.strip().split(delim)
        u = add_name(tok[st])
        v = add_name(tok[st+1])

        I.append(u)
        J.append(v)
    
    n = max(max(I), max(J)) + 1
    m = len(I)
    
    dim_f.write("p sp %d %d\n" % (n, m))
    for i in range(m):
        dim_f.write("e %d %d %d\n" % (I[i]+1, J[i]+1, 1))

    for k in name_to_index:
        dict_f.write("%s,%s\n" % (k, name_to_index[k]))
    
def csv_out(bridge_f, dict_f):
    index_to_name = {}
    for line in dict_f:
        v, k = line.strip().split(',')
        index_to_name[k] = v
    i = 0
    for line in bridge_f:
        score = line.strip()
        name = index_to_name[str(i)]
        print name + "," + score
        i += 1

def main():
    if sys.argv[1] == "-in":
        csv_in(open(sys.argv[2], 'r'), open(sys.argv[3], 'w'), open(sys.argv[4], 'w'))
    elif sys.argv[1] == "-out":
        csv_out(open(sys.argv[2]), open(sys.argv[3]))
    else:
        print "Error! Usage: python util.py {-in graph.csv output.dim output.dict,-out bridging.out output.dict}"
        exit(1)

        
if __name__ == "__main__":
    main()
