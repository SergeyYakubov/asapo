from __future__ import print_function

import asapo_producer
import sys
import time
import numpy as np
import threading
from datetime import datetime


lock = threading.Lock()

stream = sys.argv[1]
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


def callback(header, err):
    lock.acquire()  # to print
    if isinstance(err, asapo_producer.AsapoServerWarning):
        print("successfuly sent, but with warning from server: ", header, err)
    elif err is not None:
        print("could not sent: ", header, err)
    else:
        print("successfuly sent: ", header)
    lock.release()


producer = asapo_producer.create_producer(endpoint,'processed', beamtime, 'auto', stream, token, nthreads, 60)

producer.set_log_level("debug")

# send single file
producer.send_file(1, local_path="./file1", exposed_path="processed/" + stream + "/" + "file1",
                   user_meta='{"test_key":"test_val"}', callback=callback)

# send single file without callback
producer.send_file(10, local_path="./file1", exposed_path="processed/" + stream + "/" + "file10",
                   user_meta='{"test_key":"test_val"}', callback=None)

# send subsets
producer.send_file(2, local_path="./file1", exposed_path="processed/" + stream + "/" + "file2", subset=(2, 2),
                   user_meta='{"test_key":"test_val"}', callback=callback)
producer.send_file(3, local_path="./file1", exposed_path="processed/" + stream + "/" + "file3", subset=(2, 2),
                   user_meta='{"test_key":"test_val"}', callback=callback)

# send meta only
producer.send_file(3, local_path="./not_exist", exposed_path="./whatever",
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

data = np.arange(10, dtype=np.float64)

# send data from array
producer.send_data(4, "processed/" + stream + "/" + "file5", data,
                   ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)

# send data from string
producer.send_data(5, "processed/" + stream + "/" + "file6", b"hello",
                   ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)

# send metadata only
producer.send_data(6, "processed/" + stream + "/" + "file7", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

# send single file/wrong filename
producer.send_file(1, local_path="./file2", exposed_path="processed/" + stream + "/" + "file1", callback=callback)

x = np.array([[1, 2, 3], [4, 5, 6]], np.float32)
producer.send_data(8, "processed/" + stream + "/" + "file8", x,
                   ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)

try:
    x = x.T
    producer.send_data(8, "processed/" + stream + "/" + "file8", x,
                       ingest_mode=asapo_producer.DEFAULT_INGEST_MODE, callback=callback)
except asapo_producer.AsapoWrongInputError as e:
    print(e)
else:
    print("should be error sending non-cont array")
    sys.exit(1)

try:
    producer.send_file(0, local_path="./not_exist", exposed_path="./whatever",
                       ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)
except asapo_producer.AsapoWrongInputError as e:
    print(e)
else:
    print("should be error sending id 0 ")
    sys.exit(1)

# send to another substream
producer.send_data(1, "processed/" + stream + "/" + "file9", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, substream="stream", callback=callback)

# wait normal requests finished before sending duplicates

producer.wait_requests_finished(50000)

# send single file once again
producer.send_file(1, local_path="./file1", exposed_path="processed/" + stream + "/" + "file1",
                   user_meta='{"test_key":"test_val"}', callback=callback)
# send metadata only once again
producer.send_data(6, "processed/" + stream + "/" + "file7", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

# send same id different data
producer.send_file(1, local_path="./file1", exposed_path="processed/" + stream + "/" + "file1",
                   user_meta='{"test_key1":"test_val"}', callback=callback)  # send same id different data
producer.send_data(6, "processed/" + stream + "/" + "file8", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, callback=callback)

# send same id without writing to database, should success
producer.send_file(1, local_path="./file1", exposed_path="processed/" + stream + "/" + "file18",
                   user_meta='{"test_key1":"test_val"}',
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_DATA | asapo_producer.INGEST_MODE_STORE_IN_FILESYSTEM,callback=callback)

producer.wait_requests_finished(50000)
n = producer.get_requests_queue_size()
assert_eq(n, 0, "requests in queue")

# send to another data to substream stream
producer.send_data(2, "processed/" + stream + "/" + "file10", None,
                   ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY, substream="stream", callback=callback)

producer.wait_requests_finished(50000)
n = producer.get_requests_queue_size()
assert_eq(n, 0, "requests in queue")

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
assert_eq(info['lastId'], 2, "last id from different substream")

info_last = producer.last_stream()
assert_eq(info_last['name'], "stream", "last stream")
assert_eq(info_last['timestampCreated'] <= info_last['timestampLast'], True, "last is later than first")

# create with error
try:
    producer = asapo_producer.create_producer(endpoint,'processed', beamtime, 'auto', stream, token, 0, 0)
except asapo_producer.AsapoWrongInputError as e:
    print(e)
else:
    print("should be error")
    sys.exit(1)

print('Finished successfully')
