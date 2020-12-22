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

source, beamtime, token = sys.argv[1:]

consumer = asapo_consumer.create_consumer(source,".",True, beamtime,"",token,timeout)
producer  = asapo_producer.create_producer(source,'processed',beamtime,'auto', "", token, 1, 600)
producer.set_log_level("debug")

group_id  = consumer.generate_group_id()

n_send = 10

for i in range(n_send):
    producer.send_data(i+1, "name"+str(i),None,ingest_mode = asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY,stream = "stream", callback = callback)

producer.send_stream_finished_flag("stream", 10, next_stream = "next_stream", callback = callback)
producer.wait_requests_finished(timeout)

n_recv = 0
stream_finished=False
while True:
    try:
        data, meta = consumer.get_next(group_id,stream = "stream", meta_only=True)
        print ("received: ",meta)
        n_recv = n_recv + 1
    except  asapo_consumer.AsapoStreamFinishedError as finished_stream:
        stream_finished = True
        assert_eq(finished_stream.id_max, 11, "last id")
        assert_eq(finished_stream.next_stream, "next_stream", "next stream")
        break

assert_eq(n_recv, n_send, "send=recv")
assert_eq(stream_finished, True, "stream finished")
print('Using connection type: ' + consumer.current_connection_type())


