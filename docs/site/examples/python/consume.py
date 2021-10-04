import asapo_consumer

endpoint = "localhost:8400"
beamtime = "asapo_test"

# test token. In production it is created during the start of the beamtime
token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9." +
"eyJleHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmN" +
"DNhbGZwOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdGVzdC" +
"IsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGVzIjpbInd" +
"yaXRlIiwicmVhZCJdfX0.dkWupPO-ysI4t-jtWiaElAzD" +
"yJF6T7hu_Wz_Au54mYU"

# set it  according to your configuration.
path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test"


consumer = asapo_consumer \
                .create_consumer(endpoint,
                                 path_to_files,
                                 True,           # True if the path_to_files is accessible locally, False otherwise
                                 beamtime,       # Same as for the producer
                                 "test_source",  # Same as for the producer
                                 token,          # Access token
                                 5000)           # Timeout. Do not change.


streamList = consumer.get_stream_list()
for stream in streamList:
    print(stream['name'],       # the name of the stream. 'default' by default.
          stream['lastId'],     # id of the last message in stream
          stream['finished'],   # is the stream finished
          stream['nextStream']) # if the stream is finished, the next stream can be set


group_id = consumer.generate_group_id() # Several consumers can use the same group_id to process messages in parallel

try:

    # get_next is the main function to get messages from streams. You would normally call it in loop.
    # you can either manually compare the meta['_id'] to the stream['lastId'], or wait for the exception to happen
    while True:
        data, meta = consumer.get_next(group_id, meta_only = False)
        print(data.tobytes().decode("utf-8"), meta)

except asapo_consumer.AsapoStreamFinishedError:
    print('stream finished') # all the messages in the stream were processed
        
except asapo_consumer.AsapoEndOfStreamError:
    print('stream empty')    # wrong or empty stream

consumer.delete_stream(error_on_not_exist = True) # you can delete the stream after consuming
