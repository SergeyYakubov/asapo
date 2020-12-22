from __future__ import print_function

import asapo_consumer
import sys

source, path, beamtime, token = sys.argv[1:]
consumer = asapo_consumer.create_consumer(source,path,False, beamtime,"",token,1000)
group_id = consumer.generate_group_id()

res = consumer.query_images("_id > 0", stream="1")

print(res)

#data, meta = consumer.get_by_id(5,group_id,"default", meta_only=False)

#meta["buf_id"]=0
#data = consumer.retrieve_data(meta)

#print (meta)
#print (len(data),data[0:100])
#data.tofile("out")

sys.exit(0)
