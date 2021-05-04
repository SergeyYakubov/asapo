from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np
import threading
from datetime import datetime


lock = threading.Lock()

data_source = sys.argv[1]
beamtime = sys.argv[2]
endpoint = sys.argv[3]

token = ""
nthreads = 8


def assert_eq(val, expected, name):
    print("asserting eq for " + name)
    if val != expected:
        print("error at " + name)
        print('val: ', val, ' expected: ', expected)
        sys.exit(1)

class CallBackClass:
    def callback(self, payload, err):
        callback(payload,err)

callback_object = CallBackClass()

def callback(payload, err):
    lock.acquire()  # to print
    if isinstance(err, asapo_producer.AsapoServerWarning):
        print("successfuly sent, but with warning from server: ", payload, err)
    elif err is not None:
        print("could not sent: ", payload, err)
    else:
        print("successfuly sent: ", payload)
    lock.release()

def assert_version(version):
    print("asserting version ",version)
    ok = version['supported'] and version['client'] and version['server']
    if not ok:
        sys.exit(1)

producer = asapo_producer.create_producer(endpoint,'processed', beamtime, 'auto', data_source, token, nthreads, 60000)

producer.set_log_level("debug")


version = producer.get_version_info()
assert_version(version)


# send single file
producer.send_file(1, local_path="./file1", exposed_path="processed/" + data_source + "/" + "file1",
                   user_meta='{"test_key":"test_val"}', callback=callback)

# send single file without callback
producer.send_file(10, local_path="./file1", exposed_path="processed/" + data_source + "/" + "file10",
                   user_meta='{"test_key":"test_val"}', callback=None)

# send datasets
producer.send_file(2, local_path="./file1", exposed_path="processed/" + data_source + "/" + "file2", dataset=(1, 2),
                   user_meta='{"test_key":"test_val"}', callback=callback)
producer.send_file(2, local_path="./file1", exposed_path="processed/" + data_source + "/" + "file3", dataset=(2, 2),
                   user_meta='{"test_key":"test_val"}', callback=callback)

# send meta only
producer.send_file(3, local_path="./not_exist", exposed_path="./whatever",
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

data = np.arange(10, dtype=np.float64)

# send data from array
producer.send(4, "processed/" + data_source + "/" + "file5", data,
                   ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)

# send data from string
producer.send(5, "processed/" + data_source + "/" + "file6", b"hello",
                   ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)

# send metadata only
producer.send(6, "processed/" + data_source + "/" + "file7", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

# send single file/wrong filename
producer.send_file(1, local_path="./file2", exposed_path="processed/" + data_source + "/" + "file1", callback=callback)

x = np.array([[1, 2, 3], [4, 5, 6]], np.float32)
producer.send(8, "processed/" + data_source + "/" + "file8", x,
                   ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)

try:
    x = x.T
    producer.send(8, "processed/" + data_source + "/" + "file8", x,
                       ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)
except asapo_producer.AsapoWrongInputError as e:
    print(e)
else:
    print("should be error sending non-cont array")
    sys.exit(1)

try:
    producer.send(0, "processed/" + data_source + "/" + "file6", b"hello",
                       ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)
except asapo_producer.AsapoWrongInputError as e:
    print(e)
else:
    print("should be error sending id 0 ")
    sys.exit(1)

# send to another stream
producer.send(1, "processed/" + data_source + "/" + "file9", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, stream="stream", callback=callback)

# wait normal requests finished before sending duplicates

producer.wait_requests_finished(50000)

# send single file once again
producer.send_file(1, local_path="./file1", exposed_path="processed/" + data_source + "/" + "file1",
                   user_meta='{"test_key":"test_val"}', callback=callback)
# send metadata only once again
producer.send(6, "processed/" + data_source + "/" + "file7", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

# send same id different data
producer.send_file(1, local_path="./file1", exposed_path="processed/" + data_source + "/" + "file1",
                   user_meta='{"test_key1":"test_val"}', callback=callback)  # send same id different data
producer.send(6, "processed/" + data_source + "/" + "file8", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

# send same id without writing to database, should success
producer.send_file(1, local_path="./file1", exposed_path="processed/" + data_source + "/" + "file18",
                   user_meta='{"test_key1":"test_val"}',
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_DATA | asapo_producer.INGEST_MODE_STORE_IN_FILESYSTEM,callback=callback)

producer.wait_requests_finished(50000)
n = producer.get_requests_queue_size()
assert_eq(n, 0, "requests in queue")
assert_eq(n, 0, "requests in queue")

# send to another data to stream stream
producer.send(2, "processed/" + data_source + "/" + "file10", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, stream="stream", callback=callback)

producer.wait_requests_finished(50000)
n = producer.get_requests_queue_size()
assert_eq(n, 0, "requests in queue")

# pool limits (checking volume only)
data = np.arange(1000000, dtype=np.float64)
producer.set_requests_queue_limits(0,1)
try:
    producer.send(11, "processed/bla", data)
except asapo_producer.AsapoRequestsPoolIsFull as e:
    print(e)
else:
    print("should be AsapoRequestsPoolIsFull error ")
    sys.exit(1)

#stream_finished
producer.wait_requests_finished(10000)
producer.send_stream_finished_flag("stream", 2, next_stream = "next_stream", callback = callback)
# check callback_object.callback works, will be duplicated request
producer.send_stream_finished_flag("stream", 2, next_stream = "next_stream", callback = callback_object.callback)
producer.wait_requests_finished(10000)


#stream infos
info = producer.stream_info()
assert_eq(info['lastId'], 10, "stream_info last id")
assert_eq(info['name'], "default", "stream_info name")
assert_eq(info['timestampCreated']/1000000000<time.time(),True , "stream_info time created")
assert_eq(info['timestampCreated']/1000000000>time.time()-10,True , "stream_info time created")
assert_eq(info['timestampLast']/1000000000<time.time(),True , "stream_info time last")
assert_eq(info['timestampLast']/1000000000>time.time()-10,True , "stream_info time last")
print("created: ",datetime.utcfromtimestamp(info['timestampCreated']/1000000000).strftime('%Y-%m-%d %H:%M:%S.%f'))
print("last record: ",datetime.utcfromtimestamp(info['timestampLast']/1000000000).strftime('%Y-%m-%d %H:%M:%S.%f'))

info = producer.stream_info('stream')
assert_eq(info['lastId'], 3, "last id from different stream")
assert_eq(info['finished'], True, "stream finished")

info_last = producer.last_stream()
assert_eq(info_last['name'], "stream", "last stream")
assert_eq(info_last['timestampCreated'] <= info_last['timestampLast'], True, "last is later than first")

# create with error
try:
    producer = asapo_producer.create_producer(endpoint,'processed', beamtime, 'auto', data_source, token, 0, 0)
except asapo_producer.AsapoWrongInputError as e:
    print(e)
else:
    print("should be error")
    sys.exit(1)

print('Finished successfully')
