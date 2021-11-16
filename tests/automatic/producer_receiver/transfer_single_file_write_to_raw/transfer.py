from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np
import threading
from datetime import datetime


lock = threading.Lock()

data_source = sys.argv[1]
beamtime = sys.argv[2]
endpoint = sys.argv[3]

token = ""
nthreads = 8

def callback(payload, err):
    lock.acquire()  # to print
    if isinstance(err, asapo_producer.AsapoServerWarning):
        print("successfuly sent, but with warning from server: ", payload, err)
    elif err is not None:
        print("could not sent: ", payload, err)
    else:
        print("successfuly sent: ", payload)
    lock.release()

producer = asapo_producer.create_producer(endpoint,'raw', beamtime, 'auto', data_source, token, nthreads, 60000)
producer.set_log_level("debug")

# send single file
producer.send_file(1, local_path="./file1", exposed_path="raw/" + data_source + "/" + "file1", callback=callback)
producer.send_file(2, local_path="./file1", exposed_path="raw/" + data_source + "/" + "file1",
                   ingest_mode= asapo_producer.DEFAULT_INGEST_MODE|asapo_producer.INGEST_MODE_WRITE_RAW_DATA_TO_OFFLINE_FS, callback=callback)

producer.wait_requests_finished(50000)

print('Finished successfully')
