from __future__ import print_function

import asapo_worker
import sys
import json

broker, err = asapo_worker.create_server_broker("psana002:8400", "/tmp", "asapo_test2",
                                                "yzgAcLmijSLWIm8dBiGNCbc0i42u5HSm-zR6FRqo__Y=", 1000000)

if not broker:
    print("Cannot create broker: " + err)
    sys.exit(1)

last_id = 0
while True:
      array, meta, err = broker.get_last(meta_only=False)
      id = meta['_id']
      if id != last_id:
        print ("file content:",array.tostring().strip().decode("utf-8"))
        last_id = id
