#!/usr/bin/env bash

database_name=data_stream
substream=substream

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
	kill -9 $brokerid
}

echo "db.data_${substream}.insert({"_id":2})" | mongo ${database_name}
echo "db.data_${substream}.insert({"_id":1})" | mongo ${database_name}

token=`$2 token -secret auth_secret.key data`

$1 -config settings.json &

sleep 0.3
brokerid=`echo $!`

groupid=`curl -d '' --silent 127.0.0.1:5005/creategroup`
curl -v  --silent 127.0.0.1:5005/database/data/stream/${substream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"_id":1'
curl -v  --silent 127.0.0.1:5005/database/data/stream/${substream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"_id":2'
curl -v  --silent 127.0.0.1:5005/database/data/stream/${substream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"id_max":2'

# with a new group
groupid=`curl -d '' --silent 127.0.0.1:5005/creategroup`
curl -v  --silent 127.0.0.1:5005/database/data/stream/${substream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr | grep '"_id":1'