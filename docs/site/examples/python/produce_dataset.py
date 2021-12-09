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

producer = asapo_producer.create_producer(endpoint, 'processed', beamtime, 'auto', 'test_source', '', 1, 60000)

# dataset snippet_start
#assuming we have three different producers for a single dataset

# add the additional 'dataset' paremeter, which should be (<part_number>, <total_parts_in_dataset>)
producer.send(1, "processed/test_file_dataset_1", b"hello dataset 1", dataset = (1,3), callback = callback)
# this can be done from different producers in any order
producer.send(1, "processed/test_file_dataset_1", b"hello dataset 2", dataset = (2,3), callback = callback)
producer.send(1, "processed/test_file_dataset_1", b"hello dataset 3", dataset = (3,3), callback = callback)
# dataset snippet_end

producer.wait_requests_finished(2000)
# the dataset parts are not counted towards the number of messages in the stream
# the last message id in this example is still 1
producer.send_stream_finished_flag("default", 1)
