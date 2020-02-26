from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np
import threading
lock = threading.Lock()
import json

beamline = sys.argv[1]
token = sys.argv[2]
stream = sys.argv[3]
endpoint = sys.argv[4]

nthreads = 1

def callback(header,err):
    lock.acquire() # to print
    if isinstance(err,asapo_producer.AsapoServerWarning):
        print("successfuly sent, but with warning from server: ",header,err)
    elif err is not None:
        print("could not sent: ",header,err)
    else:
        print ("successfuly sent: ",header)
    lock.release()


producer  = asapo_producer.create_producer(endpoint,'auto',beamline, stream, token, nthreads, 60)

producer.set_log_level("debug")

#send single file
producer.send_file(1, local_path = "./file1", exposed_path = stream+"/"+"file1", user_meta = '{"test_key":"test_val"}', callback = callback)

producer.wait_requests_finished(10000)

time.sleep(2)

#send single file to other beamtime - should be warning on duplicated request (same beamtime, no reauthorization)
producer.send_file(1, local_path = "./file1", exposed_path = stream+"/"+"file1", user_meta = '{"test_key":"test_val"}', callback = callback)
producer.wait_requests_finished(10000)


fname = 'beamline/p07/current/beamtime-metadata-11111111.json'
with open(fname) as json_file:
    data = json.load(json_file)
data['beamtimeId']='22222222'
data['core-path']=data['core-path'].replace('11111111','22222222')

with open(fname, 'w') as outfile:
    json.dump(data, outfile)

time.sleep(2)

#send single file to other beamtime - now ok since receiver authorization timed out
producer.send_file(1, local_path = "./file1", exposed_path = stream+"/"+"file1", user_meta = '{"test_key":"test_val"}', callback = callback)

producer.wait_requests_finished(10000)

data['beamtimeId']='11111111'
data['core-path']=data['core-path'].replace('22222222','11111111')

with open(fname, 'w') as outfile:
    json.dump(data, outfile)


n = producer.get_requests_queue_size()
if n!=0:
    print("number of remaining requests should be zero, got ",n)
    sys.exit(1)





