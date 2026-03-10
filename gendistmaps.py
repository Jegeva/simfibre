#! /bin/env python3

import sys
import math

sz=(int(sys.argv[1]),int(sys.argv[2]))
center=((sz[0]-1)/2,(sz[1]-1)/2)
i=0
j=0
print("unsigned char distctr[%d]={" % (sz[0]*sz[1]))

distctr=[0 for b in range(sz[0]*sz[1])]
distQctr=[0 for b in range(sz[0]*sz[1]*16)]
distcrnr=[0 for b in range(sz[0]*sz[1]*16)]

i=0
j=0
MapMax=math.floor((math.sqrt( abs((i-center[0]))**2 + abs((j-center[1]))**2)))
for i in range(sz[0]):
    print("")
    for j in range(sz[1]):
        distctr[i*sz[0]+j] = math.floor((math.sqrt( abs((i-center[0]))**2 + abs((j-center[1]))**2)))* (255/MapMax)
        print("%d, " % (distctr[i*sz[0]+j]),end='')
print("};\n")

centerQ=((4*sz[0]-1)/2,(4*sz[1]-1)/2)
print("unsigned char distQuadctr[%d]={" % (sz[0]*sz[1]*16))
i=0
j=0
MapMax=math.floor(math.sqrt( abs((i-centerQ[0]))**2 + abs((j-centerQ[1]))**2))
for i in range(sz[0]*4):
    print("")
    for j in range(sz[1]*4):
        distQctr[i*sz[0]+j] = math.floor(math.sqrt( abs((i-centerQ[0]))**2 + abs((j-centerQ[1]))**2)) * (255/MapMax)
        print("%d, " % (distQctr[i*sz[0]+j]),end='')
print("};\n")

print("unsigned char invdistctr[%d]={" % (sz[0]*sz[1]))
for i in range(sz[0]):
    print("")
    for j in range(sz[1]):
        print("%2d, " % ( 255-distctr[i*sz[0]+j] ),end='')
print("};\n")

i=sz[0]
j=sz[1]
MapMax=math.floor(math.sqrt( abs((i)**2 + abs((j))**2)))
print("unsigned char distcornerr[%d]={" % (4*sz[0]*sz[1]))
for i in range(sz[0]):
    print("")
    for j in range(sz[1]):
        distcrnr[i*sz[0]+j] = math.ceil((math.sqrt( abs(i)**2 + abs(j)**2))) * (255/MapMax)
        print("%d, " % (distcrnr[i*sz[0]+j]),end='')
print("")

for i in range(sz[0]):
    print("")
    for j in range(sz[1]):
        print("%d, " % (distcrnr[i*sz[0]+(sz[1]-(j+1))] ),end='')
print("")
for i in range(sz[0]):
    print("")
    for j in range(sz[1]):
        print("%d, " % (distcrnr[(sz[0]-(i+1))*sz[0]+(sz[1]-(j+1))] ),end='')


print("")
for i in range(sz[0]):
    print("")
    for j in range(sz[1]):
        print("%d, " % (distcrnr[(sz[0]-(i+1))*sz[0]+j] ),end='')


print("")
print("};")
 
