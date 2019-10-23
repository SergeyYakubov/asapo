from mpi4py import MPI
import os
comm = MPI.COMM_WORLD
rank = comm.Get_rank()

with open('asapo_host', 'r') as file:
    host = file.read().replace('\n', '')

token="KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc="

os.system("hostname")
os.system("./getnext_broker "+host+":8400 /gpfs/petra3/scratch/yakubov/asapo_shared/test/asapo_test asapo_test%stream"+str(rank)+" 8 "+token+" 1000 0")

