#!/bin/sh

for i in `seq 1 9`
do
	c=`echo $i / 10 | bc -l`
	s=0.05
	/home/shweta.jain/build/scratch/igraph --confFile=RTBarabasi.conf --topoFile=test_${i}.topo --tracing=false --nix=false --percClients=$c --percServers=$s &
done
