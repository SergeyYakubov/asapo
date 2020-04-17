from __future__ import print_function

import asapo_consumer
import asapo_producer
import sys
import threading

lock = threading.Lock()

timeout = 10 * 1000

def assert_eq(val,expected,name):
    if val != expected:
        print ("error at "+name)
        print ('val: ', val,' expected: ',expected)
        sys.exit(1)

def callback(header,err):
    lock.acquire() # to print
    if err is not None:
        print("could not sent: ",header,err)
    else:
        print ("successfuly sent: ",header)
    lock.release()

source, network_type, beamtime, token = sys.argv[1:]

broker = asapo_consumer.create_server_broker(source,".",True, beamtime,"",token,timeout,network_type)
producer  = asapo_producer.create_producer(source,beamtime,'auto', "", token, 1, 600)
producer.set_log_level("debug")

group_id  = broker.generate_group_id()

n_send = 10

for i in range(n_send):
    producer.send_data(i+1, "name"+str(i),None,ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY,substream = "substream", callback = callback)

producer.send_substream_finished_flag("substream", 10, next_substream = "next_substream", callback = callback)
producer.wait_requests_finished(timeout)

n_recv = 0
substream_finished=False
while True:
    try:
        data, meta = broker.get_next(group_id,substream = "substream", meta_only=True)
        print ("received: ",meta)
        n_recv = n_recv + 1
    except  asapo_consumer.AsapoStreamFinishedError as finished_substream:
        substream_finished = True
        assert_eq(finished_substream.id_max, 11, "last id")
        assert_eq(finished_substream.next_substream, "next_substream", "next substream")
        break

assert_eq(n_recv, n_send, "send=recv")
assert_eq(substream_finished, True, "substream finished")


