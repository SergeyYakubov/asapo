#!/usr/bin/env bash

database_name=data_source
stream=stream

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
	kill -9 $brokerid
}

echo "db.data_${stream}.insert({"_id":2})" | mongo ${database_name}
echo "db.data_${stream}.insert({"_id":1})" | mongo ${database_name}

token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2MzA5MzU5MDEsImp0aSI6ImMxNGNxbmJpcHQzZGY2bDRvNHIwIiwic3ViIjoiYnRfdGVzdCIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJyZWFkIn19.D71Gv2AwSPIEkaeejWXs70sSoQzvKDonrTmtPk2J9AI

$1 -config settings.json &

sleep 0.3
brokerid=`echo $!`

groupid=`curl -d '' --silent 127.0.0.1:5005/creategroup`
curl -v  --silent 127.0.0.1:5005/database/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"_id":1'
curl -v  --silent 127.0.0.1:5005/database/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"_id":2'
curl -v  --silent 127.0.0.1:5005/database/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr  | grep '"id_max":2'

# with a new group
groupid=`curl -d '' --silent 127.0.0.1:5005/creategroup`
curl -v  --silent 127.0.0.1:5005/database/data/source/${stream}/${groupid}/next?token=$token --stderr - | tee /dev/stderr | grep '"_id":1'