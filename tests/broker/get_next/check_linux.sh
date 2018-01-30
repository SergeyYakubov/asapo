#!/usr/bin/env bash

database_name=data

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.data.deleteMany({})" | mongo ${database_name}
	echo "db.current_location.deleteMany({})" | mongo ${database_name}
	kill -9 $brokerid
}

echo "db.data.insert({"_id":2})" | mongo ${database_name}
echo "db.data.insert({"_id":1})" | mongo ${database_name}


$@ &

sleep 0.3
brokerid=`echo $!`

curl -v  --silent 127.0.0.1:5005/database/data/next --stderr - | grep '"_id":1'
curl -v  --silent 127.0.0.1:5005/database/data/next --stderr - | grep '"_id":2'

curl -v  --silent 127.0.0.1:5005/database/data/next --stderr - | grep "No Content"
