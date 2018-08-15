#!/usr/bin/env bash

set -e

trap Cleanup EXIT

function wait_mongo {
NEXT_WAIT_TIME=0
until mongo --port 27016 --eval "db.version()" | tail -2 | grep version || [ $NEXT_WAIT_TIME -eq 30 ]; do
  echo "Wait for mongo"
  NEXT_WAIT_TIME=$(( NEXT_WAIT_TIME++ ))
  sleep 1
done
if (( NEXT_WAIT_TIME == 30 )); then
    echo "Timeout"
    exit -1
fi
}


function kill_mongo {
    kill -9 `ps xa | grep mongod | grep 27016 | awk '{print $1;}'`
}


database_name=db_test
beamtime_id=asapo_test
beamline=test

Cleanup() {
	echo cleanup
    nomad stop receiver
    nomad stop discovery
    nomad stop authorizer
    nomad stop nginx
    echo "db.dropDatabase()" | mongo --port 27016 ${beamtime_id}
    kill_mongo
}

mongod --dbpath /tmp/mongo --port 27016 --logpath /tmp/mongolog --fork
wait_mongo


# create db before worker starts reading it. todo: git rid of it
echo "db.${beamtime_id}.insert({dummy:1})" | mongo --port 27016 ${beamtime_id}

sed -i 's/27017/27016/g' receiver.json.tpl
sed -i 's/"WriteToDisk":true/"WriteToDisk":false/g' receiver.json.tpl


nomad run authorizer.nmd
nomad run nginx.nmd
nomad run receiver.nmd
nomad run discovery.nmd

sleep 1

nfiles=1000

$1 localhost:8400 ${beamtime_id} 100 $nfiles 1  0 200 &

sleep 0.5

kill_mongo
sleep 3
mongod --dbpath /tmp/mongo --port 27016 --logpath /tmp/mongolog --fork


wait

echo "db.data.validate(true)" | mongo --port 27016 ${beamtime_id}

echo processed files:
echo "db.data.count()" | mongo --port 27016 ${beamtime_id}


echo "db.data.count()" | mongo --port 27016 ${beamtime_id} | grep $nfiles


