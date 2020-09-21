import asapo_consumer
import sys
import time

source, path, beamtime, token = sys.argv[1:]

broker = asapo_consumer.create_server_broker(
    source, path,True, beamtime, "stream", token, 1000)

group_id  = broker.generate_group_id()
print('generated group id: ', group_id)

while True:
    try:
        data, meta  = broker.get_last(group_id, meta_only=False)
        print('filename: ', meta['name'])
    except Exception as err:
        print('err: ', err)

    sys.stdout.flush()
    time.sleep(1)

