#!/usr/bin/env bash

database_name=test

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
	kill -9 $brokerid
}

echo 'db.meta.insert({"_id":0,"data":"test"})' | mongo ${database_name}

token=`$2 token -secret broker_secret.key ${database_name}`

$1 -config settings.json &

sleep 0.3
brokerid=`echo $!`

curl -v  --silent 127.0.0.1:5005/database/${database_name}/0/meta/0?token=$token --stderr - | grep '"data":"test"'
curl -v  --silent 127.0.0.1:5005/database/${database_name}/0/meta/1?token=$token --stderr - | grep 'not found'

