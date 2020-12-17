import asapo_consumer,os

import _thread
import time

# Define a function for the thread
def print_time( threadName, broker):
    while 1:
        group_id = broker.generate_group_id()
        print (group_id)

print ("consumer: ",asapo_consumer.__version__)
endpoint = "asap3-utl01.desy.de:8400"
beamtime = "asapo_test"
token = "KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc="
broker = asapo_consumer.create_server_broker(endpoint,"/gpfs/petra3/scratch/yakubov/asapo_shared/test_facility/gpfs/test/2019/data/asapo_test",False, beamtime,"",token,6000)


try:
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-1", broker, ) )
    _thread.start_new_thread( print_time, ("Thread-2", broker, ) )
except:
    print ("Error: unable to start thread")

while 1:
    pass

