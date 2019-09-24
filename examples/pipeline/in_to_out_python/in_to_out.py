from __future__ import print_function

import asapo_consumer
import asapo_producer
import json
import sys
import time

import threading
lock = threading.Lock()


n_send = 0

def callback(header,err):
    global n_send
    lock.acquire() # to print
    if err is not None:
        print("could not sent: ",header,err)
    else:
        print ("successfuly sent: ",header)
        n_send = n_send + 1
    lock.release()

def wait_send(n_files, timeout_s):
    elapsed = 0
    while elapsed < timeout_s:
        if n_send == n_files:
            break
        time.sleep(1)

def assert_err(err):
    if err is not None:
        print(err)
        sys.stdout.flush()
        sys.exit(1)


source, path, beamtime,stream_in, stream_out, token, timeout_s,nthreads, transfer_data = sys.argv[1:]
timeout_s=int(timeout_s)
nthreads=int(nthreads)
transfer_data=int(transfer_data)>0

broker = asapo_consumer.create_server_broker(source,path, beamtime,stream_in,token,timeout_s*1000)

producer, err = asapo_producer.create_producer(source,beamtime, stream_out, token, nthreads)
assert_err(err)

group_id  = broker.generate_group_id()

n_recv = 0

if transfer_data:
    ingest_mode = asapo_producer.DEFAULT_INGEST_MODE
else:
    ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY

while True:
    try:
        data, meta = broker.get_next(group_id, meta_only=not transfer_data)
        print ("received: ",meta)
        n_recv = n_recv + 1
        err = producer.send_data(meta['_id'],meta['name']+"_"+stream_out ,data,
                             ingest_mode = ingest_mode, callback = callback)
        assert_err(err)
    except  asapo_consumer.AsapoEndOfStreamError:
        break


wait_send(n_recv,timeout_s)

print ("Processed "+str(n_recv)+" file(s)")
print ("Sent "+str(n_send)+" file(s)")
