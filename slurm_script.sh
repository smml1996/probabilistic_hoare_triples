#!/bin/bash
#
#-------------------------------------------------------------
#running a shared memory (multithreaded) job over multiple CPUs
#-------------------------------------------------------------
#
#SBATCH --job-name=script-1-svcomp_java_Sequence.c.t2_mult.t2
#SBATCH --output=out-%j.log
#
#Number of CPU cores to use within one node
#SBATCH -c 1
#
#Define the number of hours the job should run.
#Maximum runtime is limited to 10 days, ie. 240 hours
#SBATCH --time=72:00:00
#
#Define the amount of RAM used by your job in GigaBytes
#In shared memory applications this is shared among multiple CPUs
#SBATCH --mem=16G
#
#Pick whether you prefer requeue or not. If you use the --requeue
#option, the requeued job script will start from the beginning,
#potentially overwriting your previous progress, so be careful.
#For some people the --requeue option might be desired if their
#application will continue from the last state.
#Do not requeue the job in the case it fails.
#SBATCH --no-requeue
#
#Do not export the local environment to the compute nodes
#SBATCH --export=NONE
unset SLURM_EXPORT_ENV
#
#Set the number of threads to the SLURM internal variable
export OMP_NUM_THREADS=1
#
#load the respective software module you intend to use
#run the respective binary through SLURM's srun

source $HOME/myenv/bin/activate
cd $HOME/qinterface/build/
echo $1
srun --cpu_bind=verbose  $HOME/qinterface/build/./main --command run --exp $1 --is_debug
deactivate