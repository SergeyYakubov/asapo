from __future__ import print_function

import asapo_producer
import time
import numpy as np
import sys

endpoint, beamtime = sys.argv[1:]

def callback(payload, err):
    if isinstance(err, asapo_producer.AsapoServerWarning):
        print("successfuly sent, but with warning from server: ", payload, err)
    elif err is not None:
        print("could not sent: ", payload, err)
    else:
        print("successfuly sent: ", payload)

producer = asapo_producer.create_producer(endpoint,'processed', beamtime, 'auto', "data_source", '', 4, 5000)
data = np.random.random((100, 100))

producer.send(1, exposed_path="processed/foo.dat", stream="foo", data=data.view(np.int8), callback=callback)
producer.wait_requests_finished(5000)
