#!/usr/bin/env bash

set -e

trap Cleanup EXIT SIGHUP SIGINT SIGTERM

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3
network_type=$4

beamtime_id=asapo_test

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


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
    kill $producerid
    kill $workerid
    echo "db.dropDatabase()" | mongo --port 27015 ${beamtime_id}_detector || echo "db.dropDatabase()" | mongo --port 27016 ${beamtime_id}_detector
    influx -execute "drop database ${monitor_database_name}"
    kill_mongo 27015 || kill_mongo 27016
    sed -i 's/27015/27017/g' discovery.json.tpl
    nomad stop discovery
    nomad run discovery.nmd
}


start_mongo 27016
wait_mongo 27016

sed -i 's/27017/27016/g' discovery.json.tpl
nomad stop discovery
nomad run discovery.nmd
nomad stop broker
nomad run broker.nmd

sleep 2

token=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -types read $beamtime_id`


echo "Start producer"
mkdir -p ${receiver_folder}
$producer_bin localhost:8400 ${beamtime_id} 100 1000 4 0 100 &
producerid=`echo $!`

wait

echo "Start consumer in $network_type mode"
$consumer_bin ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 10000 0 &> output.txt &
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
