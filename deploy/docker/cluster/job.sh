#!/usr/bin/env bash

#SBATCH --nodes=1
#SBATCH -t 00:40:00

srun --ntasks=$SLURM_JOB_NUM_NODES --ntasks-per-node=1 ./run_maxwell.sh

