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

producer = asapo_producer.create_producer(endpoint, 'processed', beamtime, 'auto', 'test_source', '', 1, 60000)
producer.set_log_level('error')

# let's start with producing a sample of 10 simple messages
for i in range(1, 11):
    producer.send(i, "processed/test_file_" + str(i), ('content of the message #' + str(i)).encode(), stream = 'default', callback = callback)

# finish the stream and set the next stream to be called 'next'
producer.send_stream_finished_flag('default', i, next_stream = 'next', callback = callback)

# populate the 'next' stream as well
for i in range(1, 6):
    producer.send(i, "processed/test_file_next_" + str(i), ('content of the message #' + str(i)).encode(), stream = 'next', callback = callback)

# we leave the 'next' stream unfinished, but the chain of streams can be of any length

producer.wait_requests_finished(2000)

consumer = asapo_consumer.create_consumer(endpoint, path_to_files, True, beamtime, "test_source", token, 5000)
group_id = consumer.generate_group_id()

# we start with the 'default' stream (the first one)
stream_name = 'default'

while True:
    try:
        data, meta = consumer.get_next(group_id, meta_only = False, stream = stream_name)
        text_data = data.tobytes().decode("utf-8")
        message_id = meta['_id']
        print('Message #', message_id, ':', text_data)
    except asapo_consumer.AsapoStreamFinishedError:
        # when the stream finishes, we look for the info on the next stream
        # first, we find the stream with our name in the list of streams
        stream = next(s for s in consumer.get_stream_list() if s['name'] == stream_name)
        # then we look if the field 'nextStream' is set and not empty
        if 'nextStream' in stream and stream['nextStream']:
            # if it's not, we continue with the next stream
            stream_name = stream['nextStream']
            print('Changing stream to the next one:', stream_name)
            continue
        # otherwise we stop
        print('stream finished')
        break
    except asapo_consumer.AsapoEndOfStreamError:
        print('stream ended')
        break
