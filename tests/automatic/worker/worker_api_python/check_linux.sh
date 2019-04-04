#!/usr/bin/env bash

source_path=dummy
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

for i in `seq 1 5`;
do
	echo 'db.data.insert({"_id":'$i',"size":100,"name":"'$i'","lastchange":1,"source":"none","buf_id":0})' | mongo ${database_name}
done

sleep 1

export PYTHONPATH=$1:${PYTHONPATH}

python worker_api.py 127.0.0.1:8400 $source_path $database_name $token_test_run





