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


function start_mongo {
    mongod --dbpath /tmp/mongo --port 27016 --logpath /tmp/mongolog --fork
}


beamtime_id=asapo_test
beamline=test

receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}

Cleanup() {
	echo cleanup
	set +e
	rm -rf ${receiver_root_folder}
  echo "db.dropDatabase()" | mongo --port 27016 ${beamtime_id}_detector
  kill_mongo
  sed -i 's/27016/27017/g' discovery.json.tpl
  nomad stop discovery
  nomad run discovery.nmd
}

start_mongo
wait_mongo


sed -i 's/27017/27016/g' discovery.json.tpl
nomad stop discovery
nomad run discovery.nmd

mkdir -p ${receiver_folder}

nfiles=1000
nrecords=1001 # nfiles + stream finished flag

$1 localhost:8400 ${beamtime_id} 100 $nfiles 1  0 200 &

sleep 0.5

kill_mongo
sleep 3
start_mongo

wait

echo "db.data_default.validate({full: true})" | mongo --port 27016 ${beamtime_id}_detector

echo processed files:
echo "db.data_default.count()" | mongo --port 27016 ${beamtime_id}_detector


echo "db.data_default.count()" | mongo --port 27016 ${beamtime_id}_detector | grep $nrecords


