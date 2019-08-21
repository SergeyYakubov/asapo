from __future__ import print_function

import asapo_producer
import sys
import time

#import threading
#lock = threading.Lock()


endpoint = "127.0.0.1:8400"
beamtime = "asapo_test1"
stream = ""
token = ""
nthreads = 8

def callback(header,err):
#    lock.acquire() # just example, don't do this if not needed
    if err is not None:
        print("could not sent: ",header,err)
    else:
        print ("successfuly sent: ",header)
#    lock.release()

producer, err = asapo_producer.create_producer(endpoint,beamtime, stream, token, nthreads)
if err is not None:
    print(err)
    sys.exit(1)

producer.set_log_level("info")

#send single file
err = producer.send_file(1, local_path = "./file1", exposed_path = "file1", user_meta = '{"test_key":"test_val"}', callback = callback)
if err is not None:
    print(err)


#send subsets
#producer.send_file(1, local_path = "./file1", exposed_path = "file1"",subset=(1,2),user_meta = '{"test_key":"test_val"}', callback = callback)
#producer.send_file(1, local_path = "./file1", exposed_path = "file1",subset=(1,2),user_meta = '{"test_key":"test_val"}', callback = callback)

#send meta only
err = producer.send_file(2, local_path = "./file2",exposed_path = "./file2",
                         injest_mode = asapo_producer.INJEST_MODE_TRANSFER_METADATA_ONLY, callback = callback)
if err is not None:
    print(err)

time.sleep(1)


