from __future__ import print_function

import asapo_worker
import json
import sys

source, path, beamtime, token, group_id = sys.argv[1:]

broker, err = asapo_worker.create_server_broker(source,path, beamtime,"",token,1000)


images,err = broker.query_images("meta.user_meta regexp 'test*' order by _id")

if err != None:
    print ('err: ', err)
else:
    print ('found images:',len(images))
    print (images[99]['meta']['user_meta'])



