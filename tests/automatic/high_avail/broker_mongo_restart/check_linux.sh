#!/usr/bin/env bash

set -e

trap Cleanup EXIT SIGHUP SIGINT SIGTERM

beamtime_id=asapo_test
token=`$3 token -secret auth_secret.key $beamtime_id`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

function wait_mongo {
NEXT_WAIT_TIME=0
until mongo --port $1 --eval "db.version()" | tail -2 | grep version || [ $NEXT_WAIT_TIME -eq 30 ]; do
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
    kill -9 `ps xa | grep mongod | grep $1 | awk '{print $1;}'`
}


function start_mongo {
    mongod --dbpath /tmp/mongo --port $1 --logpath /tmp/mongolog --fork
}


Cleanup() {
    set +e
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    kill $producerid
    kill $workerid
    echo "db.dropDatabase()" | mongo --port 27015 ${beamtime_id}_detector || echo "db.dropDatabase()" | mongo --port 27016 ${beamtime_id}_detector
    influx -execute "drop database ${monitor_database_name}"
    kill_mongo 27015 || kill_mongo 27016
    sed -i 's/27015/27017/g' discovery.json.tpl
}


sed -i 's/27017/27016/g' receiver.json.tpl
sed -i 's/27017/27016/g' discovery.json.tpl
sed -i 's/info/debug/g' broker.json.tpl

start_mongo 27016
wait_mongo 27016


nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo --port 27016 ${beamtime_id}_detector



#producer
mkdir -p ${receiver_folder}
$1 localhost:8400 ${beamtime_id} 100 1000 4 0 100 &
producerid=`echo $!`

wait

$2 ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 10000 0 &> output.txt &
workerid=`echo $!`

sleep 2

kill_mongo 27016
sed -i 's/27016/27015/g' discovery.json.tpl
nomad stop discovery
nomad run discovery.nmd


sleep 3
start_mongo 27015

wait

cat output.txt
nfiles=`cat output.txt | grep "Processed" | awk   '{print $2;}'`
test  $nfiles -ge 1000
rm output.txt