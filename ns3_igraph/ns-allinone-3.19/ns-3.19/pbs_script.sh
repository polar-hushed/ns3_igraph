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
PBS_HOME=/home/shweta.jain/ns3_igraph/branches/localization/ns3_igraph/ns-allinone-3.19/ns-3.19

cd $PBS_HOME

pmodel=("ns3::JakesPropagationLossModel"  "ns3::Kun2600MhzPropagationLossModel"  "ns3::OkumuraHataPropagationLossModel"  "ns3::ItuR1411LosPropagationLossModel"  "ns3::ItuR1411NlosOverRooftopPropagationLossModel")
for i in "${pmodel[@]}"; do
	echo $i
	echo "DONE"
        ./waf --run "scratch/wifi-ap --pmodel=$i --verbose=false" > ns3_job.out 2>&1

done

