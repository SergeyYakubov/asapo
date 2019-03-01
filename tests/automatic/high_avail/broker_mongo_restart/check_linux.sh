#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$3 token -secret broker_secret.key $beamtime_id`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

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


Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
#    kill $producerid
    echo "db.dropDatabase()" | mongo --port 27016 ${beamtime_id}
    influx -execute "drop database ${monitor_database_name}"
    kill_mongo
}

influx -execute "create database ${monitor_database_name}"

sed -i 's/27017/27016/g' receiver.json.tpl
sed -i 's/27017/27016/g' broker.json.tpl
sed -i 's/info/debug/g' broker.json.tpl

start_mongo
wait_mongo


nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

echo "db.${beamtime_id}.insert({dummy:1})" | mongo --port 27016 ${beamtime_id}



#producer
mkdir -p ${receiver_folder}
$1 localhost:8400 ${beamtime_id} 100 1000 4 0 100 &
#producerid=`echo $!`

wait

$2 ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 10000 0 &> output.txt &

sleep 2

kill_mongo
sleep 3
start_mongo

wait

cat output.txt
nfiles=`cat output.txt | grep "Processed" | awk   '{print $2;}'`
test  $nfiles -ge 1000
rm output.txt