from __future__ import print_function

import asapo_worker
import sys
import json
import time

source = "psana002:8400"
path = "/asapo_shared/asapo/data"
beamtime = "asapo_test"
token = "KmUDdacgBzaOD3NIJvN1NmKGqWKtx0DK-NyPjdpeWkc="

broker, err = asapo_worker.create_server_broker(
    source, path, beamtime, token, 1000)

group_id, err = broker.generate_group_id()
if err is not None:
    print('cannot generate group id, err: ', err)
else:
    print('generated group id: ', group_id)

while True:
    data, meta, err = broker.get_last(group_id, meta_only=False)
    if err is not None:
        print('err: ', err)
    else:
        print('filename: ', meta['name'])
    time.sleep(1)
