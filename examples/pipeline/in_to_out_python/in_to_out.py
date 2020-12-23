from __future__ import print_function

import asapo_consumer
import asapo_producer
import sys
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

source, path, beamtime,stream_in, stream_out, token, timeout_s,timeout_s_producer,nthreads, transfer_data = sys.argv[1:]
timeout_s=int(timeout_s)
timeout_s_producer=int(timeout_s_producer)
nthreads=int(nthreads)
transfer_data=int(transfer_data)>0

consumer = asapo_consumer.create_consumer(source,path, True,beamtime,stream_in,token,timeout_s*1000)

producer  = asapo_producer.create_producer(source,'processed',beamtime,'auto', stream_out, token, nthreads, 600000)

group_id  = consumer.generate_group_id()

n_recv = 0

if transfer_data:
    ingest_mode = asapo_producer.DEFAULT_INGEST_MODE
else:
    ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY

while True:
    try:
        data, meta = consumer.get_next(group_id, meta_only=not transfer_data)
        print ("received: ",meta)
        n_recv = n_recv + 1
        producer.send(meta['_id'],meta['name']+"_"+stream_out ,data,
                             ingest_mode = ingest_mode, callback = callback)
    except  asapo_consumer.AsapoEndOfStreamError:
        break
    except  asapo_producer.AsapoProducerError:
        break

producer.wait_requests_finished(timeout_s_producer*1000)

print ("Processed "+str(n_recv)+" file(s)")
print ("Sent "+str(n_send)+" file(s)")
