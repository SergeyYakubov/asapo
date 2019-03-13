from __future__ import print_function

import asapo_worker
import json
import sys

source, path, beamtime, token = sys.argv[1:]

broker, err = asapo_worker.create_server_broker(source,path, beamtime,token,1000)

_, meta, err = broker.get_next(meta_only=True)
if err != None:
    print ('err: ', err)
else:
    print ('filename: ', meta['name'])
    print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))
