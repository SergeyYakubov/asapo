#!/usr/bin/env bash

source_path=dummy
beamtime_id=test_run
data_source=detector
database_name=${beamtime_id}_${data_source}
token_test_run=$BT_TEST_RUN_TOKEN

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop discovery
    nomad stop authorizer
    nomad stop broker
  	echo "db.dropDatabase()" | mongo ${database_name}
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run authorizer.nmd
nomad run broker.nmd

for i in `seq 1 3`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":100,"name":"'$i'","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${database_name}
done

sleep 1

$@ 127.0.0.1:8400 $source_path $beamtime_id 2 $token_test_run 12000 1  | tee /dev/stderr | grep "Processed 3 file(s)"


