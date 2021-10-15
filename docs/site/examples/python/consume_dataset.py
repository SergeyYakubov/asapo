import asapo_consumer

endpoint = "localhost:8400"
beamtime = "asapo_test"

endpoint = "localhost:8400"
beamtime = "asapo_test"

token = str("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.e"
"yJleHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmNDNhbGZ"
"wOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdGVzdCIsIkV4dHJhQ"
"2xhaW1zIjp7IkFjY2Vzc1R5cGVzIjpbIndyaXRlIiwicmVhZCJ"
"dfX0.dkWupPO-ysI4t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU")

path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test"

consumer = asapo_consumer.create_consumer(endpoint, path_to_files, True, beamtime, "test_source", token, 5000)

group_id = consumer.generate_group_id()

try:

    # get_next_dataset behaves similarly to the regular get_next
    while True:
        dataset = consumer.get_next_dataset(group_id, stream = 'pipelined')
        print ('Dataset Id:', dataset['id'])
        # the initial response only contains the metadata
        # the actual content should be retrieved separately
        for metadata in dataset['content']:
            data = consumer.retrieve_data(metadata)
            print ('Part ' + str(metadata['dataset_substream']) + ' out of ' + str(dataset['expected_size']))
            print (data.tobytes().decode("utf-8"), metadata)

except asapo_consumer.AsapoStreamFinishedError:
    print('stream finished')
        
except asapo_consumer.AsapoEndOfStreamError:
    print('stream ended')
