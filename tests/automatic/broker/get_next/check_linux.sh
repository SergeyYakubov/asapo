#!/usr/bin/env bash

database_name=data_source
stream=stream

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
	kill -9 $brokerid
  nomad stop nginx
  nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
  nomad stop authorizer
}

echo "db.data_${stream}.insert({"_id":2})" | mongo ${database_name}
echo "db.data_${stream}.insert({"_id":1})" | mongo ${database_name}

token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2MzA5NDIxMjcsImp0aSI6ImMxNGViYnJpcHQzZHQ4Y2JhczUwIiwic3ViIjoiYnRfZGF0YSIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJyZWFkIn19.U776By_privbW9WbQCSTmk9hLZVTXzTWNNap1XOIFlM

nomad run nginx.nmd
nomad run authorizer.nmd
sleep 1


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