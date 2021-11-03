import asapo_consumer
import asapo_producer

import json

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

# beamtime_set snippet_start
# sample beamtime metadata. You can add any data you want, with any level of complexity
# in this example we use strings and ints, and one nested structure
beamtime_metadata = {
    'name': 'beamtime name',
    'condition': 'beamtime condition',
    'intvalue1': 5,
    'intvalue2': 10,
    'structure': {
        'structint1': 20,
        'structint2': 30
    }
}

# send the metadata
# by default the new metadata will completely replace the one that's already there
producer.send_beamtime_meta(json.dumps(beamtime_metadata), callback = callback)
# beamtime_set snippet_end

# beamtime_update snippet_start
# we can update the existing metadata if we want, by modifying the existing fields, or adding new ones
beamtime_metadata_update = {
    'condition': 'updated beamtime condition',
    'newintvalue': 15
}

# send the metadata in the 'update' mode
producer.send_beamtime_meta(json.dumps(beamtime_metadata_update), mode = 'update', callback = callback)
# beamtime_update snippet_end

# stream_set snippet_start
# sample stream metadata
stream_metadata = {
    'name': 'stream name',
    'condition': 'stream condition',
    'intvalue': 44
}

# works the same way: by default we replace the stream metadata, but update is also possible
# update works exactly the same as for beamtime, but here we will only do 'replace'
producer.send_stream_meta(json.dumps(stream_metadata), callback = callback)
# stream_set snippet_end

# message_set snippet_start
# sample message metadata
message_metadata = {
    'name': 'message name',
    'condition': 'message condition',
    'somevalue': 55
}

# the message metadata is sent together with the message itself
# in case of datasets each part has its own metadata
producer.send(1, "processed/test_file", b'hello', user_meta = json.dumps(message_metadata), stream = "default", callback = callback)
# message_set snippet_end

producer.wait_requests_finished(2000)

consumer = asapo_consumer.create_consumer(endpoint, path_to_files, True, beamtime, "test_source", token, 5000)

# beamtime_get snippet_start
# read the beamtime metadata
beamtime_metadata_read = consumer.get_beamtime_meta()

# the structure is the same as the one that was sent, and the updated values are already there
print('Name:', beamtime_metadata_read['name'])
print('Condition:', beamtime_metadata_read['condition'])
print('Updated value exists:', 'newintvalue' in beamtime_metadata_read)
print('Sum of int values:', beamtime_metadata_read['intvalue1'] + beamtime_metadata_read['intvalue2'])
print('Nested structure value', beamtime_metadata_read['structure']['structint1'])
# beamtime_get snippet_end

# stream_get snippet_start
# read the stream metadata
stream_metadata_read = consumer.get_stream_meta(stream = 'default')

# access various fields from it
print('Stream Name:', stream_metadata_read['name'])
print('Stream Condition:', stream_metadata_read['condition'])
print('Stream int value:', stream_metadata_read['intvalue'])
# stream_get snippet_end

group_id = consumer.generate_group_id()
try:
    while True:
        # message_get snippet_start
        # right now we are only interested in metadata
        data, meta = consumer.get_next(group_id, meta_only = True)
        print('Message #', meta['_id'])

        # our custom metadata is stored inside the message metadata
        message_metadata_read = meta['meta']
        print('Message Name:', message_metadata_read['name'])
        print('Message Condition:', message_metadata_read['condition'])
        print('Message int value:', message_metadata_read['somevalue'])
        # message_get snippet_end
except asapo_consumer.AsapoStreamFinishedError:
    print('stream finished')

except asapo_consumer.AsapoEndOfStreamError:
    print('stream ended')
