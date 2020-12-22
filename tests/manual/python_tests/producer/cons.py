from __future__ import print_function

import asapo_consumer
import sys

token="IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk="
source="127.0.0.1:8400"
path="/tmp/petra3/gpfs/p01/2019/data/asapo_test"
beamtime="asapo_test"

consumer = asapo_consumer.create_consumer(source,path,False, beamtime,"test",token,1000)
group_id = consumer.generate_group_id()

data, meta = consumer.get_by_id(3,group_id,"default", meta_only=False)

print (meta)
print (data.tostring() )


sys.exit(0)