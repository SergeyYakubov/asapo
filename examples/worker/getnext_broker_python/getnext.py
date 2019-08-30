from __future__ import print_function

import asapo_worker
import json
import sys

source, path, beamtime, token, group_id = sys.argv[1:]

broker, err = asapo_worker.create_server_broker(source,path, beamtime,"",token,1000)


if group_id == "new":
    group_id_new, err = broker.generate_group_id()
    if err != None:
        print ('cannot generate group id, err: ', err)
    else:
        print ('generated group id: ', group_id_new)
else:
    group_id_new = group_id

_, meta, err = broker.get_next(group_id_new, meta_only=True)
if err != None:
    print ('err: ', err)
else:
    print ('filename: ', meta['name'])
    print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))


beamtime_meta,err = broker.get_beamtime_meta()
if err != None:
    print ('error getting beamtime meta: ', err)
else:
    print ('beamtime meta: ', json.dumps(beamtime_meta, indent=4, sort_keys=True))
