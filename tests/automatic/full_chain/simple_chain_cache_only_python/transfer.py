from __future__ import print_function

import asapo_producer
import asapo_consumer
import sys


data_source = sys.argv[1]
beamtime = sys.argv[2]
endpoint = sys.argv[3]

token = sys.argv[4]
nthreads = 8

def callback(payload, err):
    if isinstance(err, asapo_producer.AsapoServerWarning):
        print("successfuly sent, but with warning from server: ", payload, err)
    elif err is not None:
        print("could not sent: ", payload, err)
    else:
        print("successfuly sent: ", payload)

producer = asapo_producer.create_producer(endpoint,'processed', beamtime, 'auto', data_source, token, nthreads, 6000)
producer.set_log_level("debug")

# send data
producer.send(1, "processed/file6", b"hello",
              ingest_mode=asapo_producer.CACHE_ONLY_INGEST_MODE, callback=callback)

producer.wait_requests_finished(50000)

consumer = asapo_consumer.create_consumer(endpoint, 'auto', False, beamtime, data_source, token, 5000)
group_id = consumer.generate_group_id()
_,meta  = consumer.get_next(group_id, meta_only = True)
data = consumer.retrieve_data(meta)
text_data = data.tobytes().decode("utf-8")
print (meta,text_data)

meta['buf_id'] = 0 #this will let asapo skip cache
try:
    data = consumer.retrieve_data(meta)
except asapo_consumer.AsapoDataNotInCacheError as err:
    print(err)
    pass
else:
    sys.exit(1)

print('Finished successfully')
