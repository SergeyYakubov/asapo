import asapo_consumer
import time

endpoint = "asap3-utl01.desy.de:8400"
beamtime = "11012171"
token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2MzU3NTMxMDksImp0aSI6ImMyOTR0NWFodHY1am9vZHVoaGNnIiwic3ViIjoiYnRfMTEwMTIxNzEiLCJFeHRyYUNsYWltcyI6eyJBY2Nlc3NUeXBlcyI6WyJyZWFkIiwid3JpdGUiXX19.kITePbv_dXY2ACxpAQ-PeQJPQtnR02bMoFrXq0Pbcm0"
datasource = 'VmFyZXgx'

consumer = asapo_consumer.create_consumer(endpoint,"/asap3/petra3/gpfs/p21.2/2021/data/11012171",False,
                                          beamtime,datasource,token,20000)
laststream=consumer.get_stream_list()[-1]["name"]
print("laststream = " + laststream)

group_id = consumer.generate_group_id()
t1=time.time()

data, meta = consumer.get_next(group_id, meta_only = False, stream=laststream)
#meta['buf_id'] = 0
#data = consumer.retrieve_data(meta)


print ("total time: %f" % (time.time()-t1))

print ('id:',meta['_id'])
print ('file name:',meta['name'])
print ('file content:',repr(data.tobytes()[:1000]))

