import asapo_consumer

endpoint = "localhost:8400"
beamtime = "asapo_test"
token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjk1NzE3MTAyMTYsImp0aSI6ImMzaXFhbGpmNDNhbGZwOHJua20wIiwic3ViIjoiYnRfYXNhcG9fdGVzdCIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGVzIjpbIndyaXRlIiwicmVhZCJdfX0.dkWupPO-ysI4t-jtWiaElAzDyJF6T7hu_Wz_Au54mYU"

path_to_files = "/var/tmp/asapo/global_shared/data/test_facility/gpfs/test/2019/data/asapo_test" #set it  according to your configuration.
consumer = asapo_consumer.create_consumer(endpoint,path_to_files,False, beamtime,"test_source",token,5000)
group_id = consumer.generate_group_id()

data, meta = consumer.get_next(group_id, meta_only = False)

print ('id:',meta['_id'])
print ('file name:',meta['name'])
print ('file content:',data.tobytes().decode("utf-8"))

#delete stream
consumer.delete_stream(error_on_not_exist = True)
print ('stream deleted')
