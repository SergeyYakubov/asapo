#!/usr/bin/env bash

database_name=test_detector

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

echo 'db.meta.insert({"_id":0,"data":"test"})' | mongo ${database_name}

token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2MzA5MzU5MDEsImp0aSI6ImMxNGNxbmJpcHQzZGY2bDRvNHIwIiwic3ViIjoiYnRfdGVzdCIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJyZWFkIn19.D71Gv2AwSPIEkaeejWXs70sSoQzvKDonrTmtPk2J9AI

nomad run nginx.nmd
nomad run authorizer.nmd
sleep 1


$1 -config settings.json &

sleep 0.3
brokerid=`echo $!`

curl -v  --silent 127.0.0.1:5005/database/test/detector/default/0/meta/0?token=$token --stderr - | tee /dev/stderr | grep '"data":"test"'
curl -v  --silent 127.0.0.1:5005/database/test/detector/default/0/meta/1?token=$token --stderr - | tee /dev/stderr | grep 'no documents'

