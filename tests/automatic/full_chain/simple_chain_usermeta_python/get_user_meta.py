from __future__ import print_function

import asapo_consumer
import sys

source, network_type, path, beamtime, token, group_id = sys.argv[1:]

broker = asapo_consumer.create_server_broker(source,path,True, beamtime,"",token,60000,network_type)

images = broker.query_images("meta.user_meta regexp 'test*' order by _id")

print ('found images:',len(images))
print (images[99]['meta']['user_meta'])
print ('Network mode:', broker.current_connection_type())



