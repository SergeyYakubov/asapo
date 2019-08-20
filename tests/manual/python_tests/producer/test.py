from __future__ import print_function

import asapo_producer
import sys
import json
import time

endpoint = "psana002:8400"
beamtime = "asapo_test1"
stream = "stream"
token = ""
nthreads = 1

producer, err = asapo_producer.create_producer(endpoint,beamtime, stream, token, nthreads)

if err is not None:
    print(err)
else:
    print(producer)

