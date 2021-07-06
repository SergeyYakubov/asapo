#!/usr/bin/env bash

database_name=data_source
stream=stream

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
}

echo "db.data_${stream}.insert({"_id":2})" | mongo ${database_name}
echo "db.data_${stream}.insert({"_id":1})" | mongo ${database_name}

token=$BT_DATA_TOKEN

broker=`curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.3`
echo found broker at $broker


groupid=`curl -d '' --silent $broker/v0.3/creategroup`
curl -v  --silent $broker/v0.3/beamtime/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"_id":1'
curl -v  --silent $broker/v0.3/beamtime/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"_id":2'
curl -v  --silent $broker/v0.3/beamtime/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"id_max":2'

# with a new group
groupid=`curl -d '' --silent $broker/v0.3/creategroup`
curl -v  --silent $broker/v0.3/beamtime/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr | grep '"_id":1'