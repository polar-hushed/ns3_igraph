#!/bin/bash
#
# ns3 serial PBS job.
#
#PBS -q production
#PBS -N serial_job
#PBS -l select=1:ncpus=1
#PBS -l place=free
#PBS -V

# Find out name of compute node
hostname

# Change to working directory
cd $PBS_O_WORKDIR

./pmodel.sh > ns3_job.out 2>&1

