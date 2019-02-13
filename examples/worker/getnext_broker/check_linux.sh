#!/usr/bin/env bash

database_name=test_run
token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad stop discovery
    nomad stop broker
	echo "db.dropDatabase()" | mongo ${database_name}
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd

for i in `seq 1 3`;
do
	echo 'db.data.insert({"_id":'$i',"size":100,"name":"'$i'","lastchange":1,"source":"none","buf_id":0})' | mongo ${database_name}
done

sleep 1

$@ 127.0.0.1:8400 $database_name 2 $token_test_run 1000 | grep "Processed 3 file(s)"



