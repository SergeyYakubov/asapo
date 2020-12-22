from __future__ import print_function

import asapo_consumer
import sys

source, path, beamtime, token, group_id = sys.argv[1:]

consumer = asapo_consumer.create_consumer(source,path,True, beamtime,"",token,60000)

images = consumer.query_images("meta.user_meta regexp 'test*' order by _id")

print ('found images:',len(images))
print (images[99]['meta']['user_meta'])
