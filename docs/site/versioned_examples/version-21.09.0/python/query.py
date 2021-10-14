import asapo_consumer
import asapo_producer

import json
from datetime import datetime, timedelta

def callback(payload,err):
    if err is not None and not isinstance(err, asapo_producer.AsapoServerWarning):
        print("could not send: ",payload,err)
    elif err is not None:
        print("sent with warning: ",payload,err)
    else:
        print("successfuly sent: ",payload)

endpoint = "localhost:8400"
beamtime = "asapo_test"

token = str("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.e"
"yJleHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmNDNhbGZ"
"wOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdGVzdCIsIkV4dHJhQ"
"2xhaW1zIjp7IkFjY2Vzc1R5cGVzIjpbIndyaXRlIiwicmVhZCJ"
"dfX0.dkWupPO-ysI4t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU")

path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test"

producer = asapo_producer.create_producer(endpoint, 'processed', beamtime, 'auto', 'test_source', '', 1, 60000)
producer.set_log_level('error')

# let's start with producing some messages with metadata
for i in range(1, 11):
    metadata = {
        'condition': 'condition #' + str(i),
        'somevalue': i * 10
    }
    producer.send(i, "processed/test_file_" + str(i), ('message #' + str(i)).encode(), user_meta = json.dumps(metadata), stream = "default", callback = callback)

producer.wait_requests_finished(2000)

consumer = asapo_consumer.create_consumer(endpoint, path_to_files, True, beamtime, "test_source", token, 5000)

# helper function to print messages
def print_messages(metadatas):
    # the query will return the list of metadatas
    for meta in metadatas:
        # for each metadata we need to obtain the actual message first
        data = consumer.retrieve_data(meta)
        print('Message #', meta['_id'], ', content:', data.tobytes().decode("utf-8"), ', usermetadata:', meta['meta'])

# simple query, same as get_by_id
metadatas = consumer.query_messages('_id = 1')
print('Message with ID = 1')
print_messages(metadatas)

# the query that requests the range of IDs
metadatas = consumer.query_messages('_id >= 8')
print('Messages with ID >= 8')
print_messages(metadatas)

# the query that has some specific requirement for message metadata
metadatas = consumer.query_messages('meta.condition = "condition #7"')
print('Message with condition = "condition #7"')
print_messages(metadatas)

# the query that has several requirements for user metadata
metadatas = consumer.query_messages('meta.somevalue > 30 AND meta.somevalue < 60')
print('Message with 30 < somevalue < 60')
print_messages(metadatas)

# the query that is based on the message's timestamp
now = datetime.now()
fifteen_minutes_ago = now - timedelta(minutes = 15)
# python uses timestamp in seconds, while ASAP::O in nanoseconds, so we need to multiply it by a billion
metadatas = consumer.query_messages('timestamp < {} AND timestamp > {}'.format(now.timestamp() * 10**9, fifteen_minutes_ago.timestamp() * 10**9))
print('Messages in the last 15 minutes')
print_messages(metadatas)

