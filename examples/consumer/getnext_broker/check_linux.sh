#!/usr/bin/env bash

source_path=dummy
beamtime_id=test_run
stream=detector
database_name=${beamtime_id}_${stream}
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
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd

for i in `seq 1 3`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":100,"name":"'$i'","timestamp":1,"source":"none","buf_id":0,"meta":{"test":10}})' | mongo ${database_name}
done

sleep 1

$@ 127.0.0.1:8400 $source_path $beamtime_id 2 $token_test_run 12000 1  | tee /dev/stderr | grep "Processed 3 file(s)"



