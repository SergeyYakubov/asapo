from __future__ import print_function

import asapo_consumer
import json
import sys

source, path, beamtime, token, group_id = sys.argv[1:]

broker = asapo_consumer.create_server_broker(source,path, beamtime,"",token,1000)


if group_id == "new":
    group_id_new = broker.generate_group_id()
    print ('generated group id: ', group_id_new)
else:
    group_id_new = group_id

_, meta = broker.get_next(group_id_new, meta_only=True)
print ('filename: ', meta['name'])
print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))

try:
    beamtime_meta = broker.get_beamtime_meta()
    print ('beamtime meta: ', json.dumps(beamtime_meta, indent=4, sort_keys=True))
except asapo_consumer.AsapoError as err:
    print ('error getting beamtime meta: ', err)
