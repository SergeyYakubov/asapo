#!/usr/bin/env bash

database_name=test_run_detector
token_test_run=$BT_TEST_RUN_TOKEN
set -e

trap Cleanup EXIT

Cleanup() {
    set +e
  	echo "db.dropDatabase()" | mongo ${database_name}
}

for i in `seq 1 10`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":100,"name":"'$i'","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

$@ 127.0.0.1:8400 test_run 4 10 $token_test_run


