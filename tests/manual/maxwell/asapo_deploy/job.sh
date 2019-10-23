#!/usr/bin/env bash

#SBATCH --nodes=5
#SBATCH -t 6-10:40:00


echo start on $SLURM_JOB_NUM_NODES
srun --ntasks=$SLURM_JOB_NUM_NODES --ntasks-per-node=1 ./run_maxwell.sh

