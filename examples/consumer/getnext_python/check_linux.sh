#!/usr/bin/env bash

source_path=dummy
beamtime_id=test_run
data_source=detector
database_name=${beamtime_id}_${data_source}
token_test_run=$BT_TEST_RUN_TOKEN
group_id=bif31l2uiddd4r0q6b40
set -e

trap Cleanup EXIT

Cleanup() {
    set +e
	echo "db.dropDatabase()" | mongo ${database_name}
}

for i in `seq 1 3`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":100,"name":"'$i'","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

echo 'db.meta.insert({"_id":"bt","meta":{"meta_test":"test"}})' | mongo ${database_name}

sleep 1

export PYTHONPATH=$1:${PYTHONPATH}
Python_EXECUTABLE=$2


$Python_EXECUTABLE getnext.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run $group_id > out
cat out
cat out | grep '"size": 100'
cat out | grep '"_id": 1'
cat out | grep '"meta_test": "test"'

$Python_EXECUTABLE getnext.py 127.0.0.1:8400 $source_path $beamtime_id $token_test_run $group_id> out
cat out
cat out | grep '"_id": 2'

#echo $?



