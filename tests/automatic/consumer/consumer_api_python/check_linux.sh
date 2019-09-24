#!/usr/bin/env bash

source_path=.
beamtime_id=test_run
stream=detector
database_name=${beamtime_id}_${stream}
token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=
set -e



trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad stop discovery
    nomad stop broker
	echo "db.dropDatabase()" | mongo ${database_name}
	rm 1 1_1
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd

echo hello1 > 1
echo hello1 > 1_1


for i in `seq 1 5`;
do
	echo 'db.data.insert({"_id":'$i',"size":6,"name":"'$i'","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}})' | mongo ${database_name}
done

sleep 1

export PYTHONPATH=$1:${PYTHONPATH}

python consumer_api.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run single


#check datasets
echo "db.dropDatabase()" | mongo ${database_name}

sleep 1

for i in `seq 1 10`;
do
	images=''
	for j in `seq 1 3`;
	do
		images="$images,{"_id":$j,"size":6,"name":'${i}_${j}',"lastchange":1,"source":'none',"buf_id":0,"meta":{"test":10}}"
	done
	images=${images#?}
	echo 'db.data.insert({"_id":'$i',"size":3,"images":['$images']})' | mongo ${database_name}
done


python consumer_api.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run datasets
