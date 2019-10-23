#!/usr/bin/env bash

#SBATCH --nodes=4
#SBATCH --ntasks=4
#SBATCH -t 00:10:00

export asapo_host=`cat asapo_host`

module load mpi/openmpi-x86_64

mpirun --map-by node --mca mpi_warn_on_fork 0 python produce.py

