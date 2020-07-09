from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np
import threading
lock = threading.Lock()


endpoint = "127.0.0.1:8400"
beamtime = "asapo_test"
stream = "test"
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

producer = asapo_producer.create_producer(endpoint,beamtime,'auto', stream, token, nthreads ,0)

producer.set_log_level("debug")

#send meta only
producer.send_file(3, local_path = "/tmp/petra3/gpfs/p01/2019/data/asapo_test",exposed_path = "producer/aaa",
                         ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback = callback)

producer.wait_requests_finished(1000)

