from mpi4py import MPI
import os
comm = MPI.COMM_WORLD
rank = comm.Get_rank()

with open('asapo_host', 'r') as file:
    host = file.read().replace('\n', '')

token="KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc="
os.system("hostname")
os.system("./dummy-data-producer "+host+":8400 asapo_test%stream"+str(rank)+"%"+token+" 1000000 50 8 0 1000")


