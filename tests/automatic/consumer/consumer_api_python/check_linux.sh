#!/usr/bin/env bash

source_path=.
beamtime_id=test_run
stream=detector
database_name=${beamtime_id}_${stream}
token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=
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
    kill -2 `ps xa | grep mongod | grep 27016 | awk '{print $1;}'`
}


function start_mongo {
    mongod --dbpath /tmp/mongo --port 27016 --logpath /tmp/mongolog --fork
}


Cleanup() {
    set +e
    nomad stop nginx >/dev/null
    nomad stop discovery >/dev/null
    nomad stop broker >/dev/null
	echo "db.dropDatabase()" | mongo --port 27016 ${database_name} >/dev/null
	rm 1 1_1
    kill_mongo
}

sed -i 's/27017/27016/g' discovery.json.tpl


start_mongo
wait_mongo

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd

echo hello1 > 1
echo hello1 > 1_1

for i in `seq 1 5`;
do
	echo 'db.data.insert({"_id":'$i',"size":6,"name":"'$i'","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}})' | mongo --port 27016 ${database_name} >/dev/null
done

sleep 1

export PYTHONPATH=$1:${PYTHONPATH}

kill_mongo
#python consumer_api.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run broker_server_error
sleep 1 
start_mongo
wait_mongo
python consumer_api.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run single



#check datasets
echo "db.dropDatabase()" | mongo --port 27016 ${database_name} > /dev/null

sleep 1

for i in `seq 1 10`;
do
	images=''
	for j in `seq 1 3`;
	do
		images="$images,{"_id":$j,"size":6,"name":'${i}_${j}',"lastchange":1,"source":'none',"buf_id":0,"meta":{"test":10}}"
	done
	images=${images#?}
	echo 'db.data.insert({"_id":'$i',"size":3,"images":['$images']})' | mongo --port 27016 ${database_name} >/dev/null
done


python consumer_api.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run datasets
