from __future__ import print_function

import asapo_producer
import time
import threading

lock = threading.Lock()

def callback(payload, err):
    lock.acquire()  # to print
    if isinstance(err, asapo_producer.AsapoServerWarning):
        print("successfuly sent, but with warning from server: ", payload, err)
    elif err is not None:
        print("could not sent: ", payload, err)
    else:
        print("successfuly sent: ", payload)
    lock.release()

producer = asapo_producer.create_producer("google.com:8700",'processed', "beamtime", 'auto', "data_source", "token", 4, 5000)


for i in range(1, 20):
    print ("sending ",i)
    producer.send_file(i, local_path="./not_exist", exposed_path="./whatever",
                       ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)
    time.sleep(1)

