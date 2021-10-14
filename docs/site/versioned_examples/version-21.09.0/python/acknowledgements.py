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
    producer.send(i, "processed/test_file_ack_" + str(i), ('message #' + str(i)).encode(), stream = "default", callback = callback)

producer.wait_requests_finished(2000)

consumer = asapo_consumer.create_consumer(endpoint, path_to_files, True, beamtime, "test_source", token, 5000)
group_id = consumer.generate_group_id()

# the flag to separate the first attempt for message #3
firstTryNegative = True

try:
    while True:
        data, meta = consumer.get_next(group_id, meta_only = False)
        text_data = data.tobytes().decode("utf-8")
        message_id = meta['_id']

        # acknowledge all the messages except these
        if message_id not in [3,5,7]:
            print('Acknowledge the message #', message_id)
            consumer.acknowledge(group_id, message_id)

        # for message #3 we issue a negative acknowledgement, which will put it at the next place in the stream
        # in this case, it will be put in the end of a stream
        if message_id == 3:
            if firstTryNegative:
                print('Negative acknowledgement of the message #', message_id)
                # make the acknowledgement with a delay of 1 second
                consumer.neg_acknowledge(group_id, message_id, delay_ms=2000)
                firstTryNegative = False
            else:
                # on our second attempt we acknowledge the message
                print('Second try of the message #', message_id)
                consumer.acknowledge(group_id, message_id)

except asapo_consumer.AsapoStreamFinishedError:
    print('stream finished')

except asapo_consumer.AsapoEndOfStreamError:
    print('stream ended')

for message_id in consumer.get_unacknowledged_messages(group_id):
    data, meta = consumer.get_by_id(message_id, meta_only = False)
    print('Unacknowledged message:', data.tobytes().decode("utf-8"), meta)
