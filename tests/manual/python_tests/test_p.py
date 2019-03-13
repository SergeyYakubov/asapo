from __future__ import print_function

import asapo_worker
import sys
import json

broker, err = asapo_worker.create_server_broker("psana002:8400", "/tmp", "asapo_test2",
                                                "yzgAcLmijSLWIm8dBiGNCbc0i42u5HSm-zR6FRqo__Y=", 1000000)

if not broker:
    print("Cannot create broker: " + err)
    sys.exit(1)

first = True
while True:
    if first:
        array, meta, err = broker.get_last(meta_only=False)
        first = False
    else:
        array, meta, err = broker.get_next(meta_only=False)
    if err:
        print ('err: ', err)
        break
    print ('data:', array.tostring().strip())
    print ('filename: ', meta['name'])
    print ('meta: ', json.dumps(meta, indent=4, sort_keys=True))
