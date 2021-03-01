#!/usr/bin/env bash

beamtime_id=test_run
data_source=detector
database_name=${beamtime_id}_${data_source}
token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop discovery
    nomad stop broker
    echo "db.dropDatabase()" | mongo ${database_name}
	rm -f 1_1 1
}


nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

for i in `seq 1 10`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":6,"name":"'$i'","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

for i in `seq 1 5`;
do
	echo 'db.data_stream1.insert({"_id":'$i',"size":6,"name":"'1$i'","timestamp":1000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

echo 'db.data_stream1.insert({"_id":'6',"size":0,"name":"asapo_finish_stream","timestamp":1000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"next_stream":"ns"}})' | mongo ${database_name}

for i in `seq 1 5`;
do
	echo 'db.data_stream2.insert({"_id":'$i',"size":6,"name":"'2$i'","timestamp":2000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done
echo 'db.data_stream2.insert({"_id":'6',"size":0,"name":"asapo_finish_stream","timestamp":2000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"next_stream":"asapo_no_next"}})' | mongo ${database_name}

echo hello1 > 1

$@ 127.0.0.1:8400 $beamtime_id $token_test_run single


#check datasets
echo "db.dropDatabase()" | mongo ${database_name}

sleep 1

for i in `seq 1 10`;
do
	messages=''
	for j in `seq 1 3`;
	do
		messages="$messages,{"_id":$j,"size":6,"name":'${i}_${j}',"timestamp":1000,"source":'none',"buf_id":0,"dataset_substream":0,"meta":{"test":10}}"
	done
	messages=${messages#?}
	echo 'db.data_default.insert({"_id":'$i',"size":3,"messages":['$messages']})' | mongo ${database_name}
done

for i in `seq 1 5`;
do
	messages=''
	for j in `seq 1 2`;
	do
		messages="$messages,{"_id":$j,"size":6,"name":'${i}_${j}',"timestamp":1000,"source":'none',"buf_id":0,"dataset_substream":0,"meta":{"test":10}}"
	done
	messages=${messages#?}
	echo 'db.data_incomplete.insert({"_id":'$i',"size":3,"messages":['$messages']})' | mongo ${database_name}
done


echo hello1 > 1_1

$@ 127.0.0.1:8400 $beamtime_id $token_test_run dataset
