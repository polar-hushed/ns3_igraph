#!/bin/bash
#
# ns3 serial PBS job.
#
#PBS -q production
#PBS -N ns3_pmodel
#PBS -l select=1:ncpus=1
#PBS -l place=free
#PBS -V

# Find out name of compute node
hostname
# Change to working directory
PBS_HOME=/home/shweta.jain/ns3_igraph/ns3_igraph/ns-allinone-3.19/ns-3.19

cd $PBS_O_WORKDIR

for i in `seq 1 9`
do
	c=`echo $i / 10 | bc -l`
	s=0.05
	/home/shweta.jain/build/scratch/igraph --confFile=RTBarabasi.conf --topoFile=test_${i}.topo --tracing=false --nix=false --percClients=$c --percServers=$s  > ns3_job.out 2>&1
done

