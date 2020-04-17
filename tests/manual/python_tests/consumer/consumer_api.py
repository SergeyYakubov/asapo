from __future__ import print_function

import asapo_consumer
import sys

source, network_type, path, beamtime, token = sys.argv[1:]
broker = asapo_consumer.create_server_broker(source,path,False, beamtime,"",token,1000,network_type)
group_id = broker.generate_group_id()


_, meta = broker.get_by_id(1,group_id, meta_only=True)

#meta["buf_id"]=0
data = broker.retrieve_data(meta)

print (meta)
print (len(data),data[0:100])
data.tofile("out")

sys.exit(0)
