from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np

data_source = sys.argv[1]
beamtime = sys.argv[2]
endpoint = sys.argv[3]

token = ""
nthreads = 1

class AsapoSender:
    def __init__(self, producer):
        self.producer = producer
        self.ingest_mode = asapo_producer.DEFAULT_INGEST_MODE
        self._n_queued = 8
    def send(self, data, metadata):
        self.producer.send(
                metadata['_id'], metadata['name'], data,
                ingest_mode=self.ingest_mode,
                callback=self._callback)
    def _callback(self, header, err):
    	print ("hello self callback")

producer  = asapo_producer.create_producer(endpoint,'auto','auto','processed',beamtime,'auto', data_source, token, nthreads, 600000)
producer.set_log_level("debug")

print(asapo_producer.__version__)

sender = AsapoSender(producer)

meta={}
meta['_id'] = 1
meta['name'] = data_source+"/"+"file1"
data = np.array([[1, 2, 3], [4, 5, 6]], np.float32)
sender.send(data, meta)

producer.wait_requests_finished(15000)
