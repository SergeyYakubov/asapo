from __future__ import print_function

import asapo_consumer
import sys

source, beamtime,path, token = sys.argv[1:]
broker = asapo_consumer.create_server_broker(source,path,False, beamtime,"",token,1000)
group_id = broker.generate_group_id()


data, meta = broker.get_by_id(1, group_id, meta_only=False)

print (meta)
print (len(data))


sys.exit(0)