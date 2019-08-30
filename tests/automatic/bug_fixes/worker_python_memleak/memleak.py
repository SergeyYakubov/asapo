import asapo_worker
import time
import sys

source, path, beamtime, token = sys.argv[1:]

broker, err = asapo_worker.create_server_broker(
    source, path, beamtime, "stream", token, 1000)

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
    sys.stdout.flush()
    time.sleep(1)
