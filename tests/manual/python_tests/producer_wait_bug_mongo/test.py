from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np
import threading
lock = threading.Lock()


endpoint = "127.0.0.1:8400"
beamtime = "asapo_test1"
data_source = "detector"
token = ""
nthreads = 8

def callback(header,err):
    lock.acquire() # to print
    if err is not None:
        print("could not sent: ",header,err)
    else:
        print ("successfuly sent: ",header)
    lock.release()

def assert_err(err):
    if err is not None:
        print(err)
        sys.exit(1)

producer = asapo_producer.create_producer(endpoint,'processed','auto','auto',beamtime,'auto', data_source, token, nthreads, 600000)

producer.set_log_level("debug")

#send single file
producer.send_file(1, local_path = "./file1", exposed_path = data_source+"/"+"file1", user_meta = '{"test_key":"test_val"}', callback = callback)


#send datasets
producer.send_file(2, local_path = "./file1", exposed_path = data_source+"/"+"file2",dataset=(2,2),user_meta = '{"test_key":"test_val"}', callback = callback)
producer.send_file(3, local_path = "./file1", exposed_path = data_source+"/"+"file3",dataset=(2,2),user_meta = '{"test_key":"test_val"}', callback = callback)

#send meta only
producer.send_file(3, local_path = "./not_exist",exposed_path = "./whatever",
                         ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback = callback)

data = np.arange(10,dtype=np.float64)

#send data from array
producer.send(4, data_source+"/"+"file5",data,
                         ingest_mode = asapo_producer.DEFAULT_INGEST_MODE, callback = callback)

#send data from string
err = producer.send(5, data_source+"/"+"file6",b"hello",
                         ingest_mode = asapo_producer.DEFAULT_INGEST_MODE, callback = callback)

#send metadata only
producer.send(6, data_source+"/"+"file7",None,
                         ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback = callback)


x = np.array([[1, 2, 3], [4, 5, 6]], np.float32)
producer.send(4, data_source+"/"+"file5",x,
                         ingest_mode = asapo_producer.DEFAULT_INGEST_MODE, callback = callback)

try:
	x = x.T
	producer.send(4, data_source+"/"+"file5",x,
                         ingest_mode = asapo_producer.DEFAULT_INGEST_MODE, callback = callback)
except:
	pass
else:
	print ("should be exception")


producer.wait_requests_finished(1000)

