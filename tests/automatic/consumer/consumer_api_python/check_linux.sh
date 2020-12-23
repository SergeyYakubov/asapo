#!/usr/bin/env bash

beamtime_id=test_run
source_path=`pwd`/asap3/petra3/gpfs/p01/2019/data/$beamtime_id
data_source=detector
database_name=${beamtime_id}_${data_source}
token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=
set -e


trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx >/dev/null
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop discovery >/dev/null
    nomad stop broker >/dev/null
    nomad stop file_transfer >/dev/null
    nomad stop authorizer >/dev/null
	echo "db.dropDatabase()" | mongo ${database_name} >/dev/null
	rm $source_path/1 $source_path/1_1
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd
nomad run file_transfer.nmd
nomad run authorizer.nmd


mkdir -p $source_path
echo -n hello1 > $source_path/1
echo -n hello1 > $source_path/1_1

for i in `seq 1 5`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":6,"name":"'$i'","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

echo 'db.data_streamfts.insert({"_id":'1',"size":0,"name":"'1'","timestamp":1000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}

for i in `seq 1 5`;
do
	echo 'db.data_stream1.insert({"_id":'$i',"size":6,"name":"'1$i'","timestamp":2000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

for i in `seq 1 5`;
do
	echo 'db.data_stream2.insert({"_id":'$i',"size":6,"name":"'2$i'","timestamp":3000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

sleep 1

export PYTHONPATH=$1:${PYTHONPATH}
export Python_EXECUTABLE=$2
$Python_EXECUTABLE $3/consumer_api.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run single

#check datasets
echo "db.dropDatabase()" | mongo ${database_name} > /dev/null

sleep 1

for i in `seq 1 10`;
do
	messages=''
	for j in `seq 1 3`;
	do
		messages="$messages,{"_id":$j,"size":6,"name":'${i}_${j}',"timestamp":0,"source":'none',"buf_id":0,"dataset_substream":0,"meta":{"test":10}}"
	done
	messages=${messages#?}
	echo 'db.data_default.insert({"_id":'$i',"size":3,"messages":['$messages']})' | mongo ${database_name} >/dev/null
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



$Python_EXECUTABLE $3/consumer_api.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run datasets
