#!/usr/bin/env bash

database_name=test_stream

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
	kill -9 $brokerid
}

echo 'db.meta.insert({"_id":0,"data":"test"})' | mongo ${database_name}

token=`$2 token -secret auth_secret.key test`

$1 -config settings.json &

sleep 0.3
brokerid=`echo $!`

curl -v  --silent 127.0.0.1:5005/database/test/stream/default/0/meta/0?token=$token --stderr - | tee /dev/stderr | grep '"data":"test"'
curl -v  --silent 127.0.0.1:5005/database/test/stream/default/0/meta/1?token=$token --stderr - | tee /dev/stderr | grep 'no documents'

