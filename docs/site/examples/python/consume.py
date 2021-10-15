import asapo_consumer

endpoint = "localhost:8400"
beamtime = "asapo_test"

# test token. In production it is created during the start of the beamtime
token = str("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.e"
"yJleHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmNDNhbGZ"
"wOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdGVzdCIsIkV4dHJhQ"
"2xhaW1zIjp7IkFjY2Vzc1R5cGVzIjpbIndyaXRlIiwicmVhZCJ"
"dfX0.dkWupPO-ysI4t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU")

# set it  according to your configuration.
path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test"


consumer = asapo_consumer \
                .create_consumer(endpoint,
                                 path_to_files,
                                 True,           # True if the path_to_files is accessible locally, False otherwise
                                 beamtime,       # Same as for the producer
                                 "test_source",  # Same as for the producer
                                 token,          # Access token
                                 5000)           # Timeout. How long do you want to wait on non-finished stream for a message.


# you can get info about the streams in the beamtime
for stream in consumer.get_stream_list():
    print("Stream name: ", stream['name'], "\n",
          "LastId: ", stream['lastId'], "\n",
          "Stream finished: ", stream['finished'], "\n",
          "Next stream: ", stream['nextStream'])


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
    print('stream ended')    # not-finished stream timeout, or wrong or empty stream

consumer.delete_stream(error_on_not_exist = True) # you can delete the stream after consuming
