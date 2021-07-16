import asapo_producer

def callback(payload,err):
    if err is not None:
        print("could not sent: ",payload,err)
    else:
        print ("successfuly sent: ",payload)

endpoint = "localhost:8400"
beamtime = "asapo_test"

# source type 'processed' to write to the core filesystem
producer = asapo_producer.create_producer(endpoint,'processed',
                                          beamtime,'auto','test_source','', 1,60000)

# we are sending a message with with index 1 to the default stream. Filename must start with processed/
producer.send(1, "processed/test_file",b"hello",
              callback = callback)

producer.wait_requests_finished(2000)
