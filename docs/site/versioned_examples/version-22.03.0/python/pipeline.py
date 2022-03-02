import asapo_consumer
import asapo_producer

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

consumer = asapo_consumer.create_consumer(endpoint, path_to_files, True, beamtime, "test_source", token, 5000)

producer = asapo_producer.create_producer(endpoint, 'processed', beamtime, 'auto', 'test_source', '', 1, 60000)

group_id = consumer.generate_group_id()
# pipeline snippet_start
# put the processed message into the new stream
pipelined_stream_name = 'pipelined'

try:
    while True:
        # we expect the message to be in the 'default' stream already
        data, meta = consumer.get_next(group_id, meta_only = False)
        message_id = meta['_id']
        
        # work on our data
        text_data = data.tobytes().decode("utf-8")
        pipelined_message = (text_data + ' processed').encode()
        
        # you may use the same filename, if you want to rewrite the source file. This will result in warning, but it is a valid usecase
        producer.send(message_id, "processed/test_file_" + message_id, pipelined_message, pipelined_stream_name, callback = callback)
        

except asapo_consumer.AsapoStreamFinishedError:
    print('stream finished')
        
except asapo_consumer.AsapoEndOfStreamError:
    print('stream ended')
# pipeline snippet_end
producer.wait_requests_finished(2000)

# finish snippet_start
# the meta from the last iteration corresponds to the last message
last_id = meta['_id']

producer.send_stream_finished_flag("pipelined", last_id)
# finish snippet_end

# you can remove the source stream if you do not need it anymore
consumer.delete_stream(stream = 'default', error_on_not_exist = True)
