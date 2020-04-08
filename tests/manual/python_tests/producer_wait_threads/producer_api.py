from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np
import threading
lock = threading.Lock()

stream = "python"
beamtime = "asapo_test"
endpoint = "127.0.0.1:8400"

token = ""
nthreads = 8

def callback(header,err):
    lock.acquire() # to print
    if err is not None:
        print("could not sent: ",header,err)
    else:
        print ("successfuly sent: ",header)
    lock.release()

producer  = asapo_producer.create_producer(endpoint,beamtime, 'auto', stream, token, nthreads, 600)

producer.set_log_level("info")

#send single file
producer.send_file(1, local_path = "./file1", exposed_path = stream+"/"+"file1", user_meta = '{"test_key":"test_val"}', callback = callback)

#send single file without callback
producer.send_file(1, local_path = "./file1", exposed_path = stream+"/"+"file1", user_meta = '{"test_key":"test_val"}',callback=None)

#send subsets
producer.send_file(2, local_path = "./file1", exposed_path = stream+"/"+"file2",subset=(2,2),user_meta = '{"test_key":"test_val"}', callback = callback)
producer.send_file(3, local_path = "./file1", exposed_path = stream+"/"+"file3",subset=(2,2),user_meta = '{"test_key":"test_val"}', callback = callback)

#send meta only
producer.send_file(3, local_path = "./not_exist",exposed_path = "./whatever",
                         ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback = callback)

data = np.arange(10,dtype=np.float64)

#send data from array
producer.send_data(4, stream+"/"+"file5",data,
                         ingest_mode = asapo_producer.DEFAULT_INGEST_MODE, callback = callback)

#send data from string
producer.send_data(5, stream+"/"+"file6",b"hello",
                         ingest_mode = asapo_producer.DEFAULT_INGEST_MODE, callback = callback)

#send metadata only
producer.send_data(6, stream+"/"+"file7",None,
                         ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback = callback)

producer.wait_requests_finished(1000)
n = producer.get_requests_queue_size()
if n!=0:
	print("number of remaining requestst should be zero, got ",n)
	sys.exit(1)


# create with error
try:
    producer  = asapo_producer.create_producer(endpoint,beamtime,'auto', stream, token, 0, 600)
except Exception as Asapo:
    print(e)
else:
    print("should be error")
    sys.exit(1)




