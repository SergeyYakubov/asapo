#!/usr/bin/env bash

database_name=data

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
	kill -9 $brokerid
}

echo "db.data.insert({"_id":2})" | mongo ${database_name}
echo "db.data.insert({"_id":1})" | mongo ${database_name}

token=`$2 token -secret broker_secret.key data`

$1 -config settings.json &

sleep 0.3
brokerid=`echo $!`


groupid=`curl -d '' --silent 127.0.0.1:5005/creategroup`

curl -v  --silent 127.0.0.1:5005/database/data/${groupid}/last?token=$token --stderr -

curl -v  --silent 127.0.0.1:5005/database/data/${groupid}/last?token=$token --stderr - | grep '"_id":2'
curl -v  --silent 127.0.0.1:5005/database/data/${groupid}/last?token=$token --stderr - | grep '"_id":2'

echo "db.data.insert({"_id":3})" | mongo ${database_name}

curl -v  --silent 127.0.0.1:5005/database/data/${groupid}/last?token=$token --stderr - | grep '"_id":3'

echo "db.data.insert({"_id":4})" | mongo ${database_name}

curl -v  --silent 127.0.0.1:5005/database/data/${groupid}/next?token=$token --stderr - | grep '"_id":4'
curl -v  --silent 127.0.0.1:5005/database/data/${groupid}/last?token=$token --stderr - | grep '"_id":4'