from __future__ import print_function

import asapo_consumer
import sys

source, path, beamtime, token, group_id = sys.argv[1:]

consumer = asapo_consumer.create_consumer(source,path,True, beamtime,"",token,60000)

messages = consumer.query_messages("meta.user_meta regexp 'test*' order by _id")

print ('found messages:',len(messages))
print (messages[99]['meta']['user_meta'])
