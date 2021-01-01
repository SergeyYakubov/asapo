import asapo_consumer
import sys
import time

source, path, beamtime, token = sys.argv[1:]

consumer = asapo_consumer.create_consumer(
    source, path,True, beamtime, "source", token, 1000)

group_id  = consumer.generate_group_id()
print('generated group id: ', group_id)

while True:
    try:
        data, meta  = consumer.get_last(group_id, meta_only=False)
        print('filename: ', meta['name'])
    except Exception as err:
        print('err: ', err)

    sys.stdout.flush()
    time.sleep(1)

