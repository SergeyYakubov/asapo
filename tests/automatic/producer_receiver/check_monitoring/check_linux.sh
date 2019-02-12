#!/usr/bin/env bash

database_name=db_test
beamtime_id=asapo_test
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}
set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	influx -execute "drop database ${database_name}"
    nomad stop receiver
    nomad stop discovery
    nomad stop authorizer
    nomad stop nginx
    echo "db.dropDatabase()" | mongo ${beamtime_id}
    rm -rf ${receiver_root_folder}
}

mkdir -p ${receiver_folder}

influx -execute "create database ${database_name}"

nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run nginx.nmd

sleep 1

$1 localhost:8400 ${beamtime_id} 100 112 4  0 100

sleep 2

# should be 116 requests (112 data transfers and 4 authorizations)
influx -execute "select sum(n_requests) from statistics" -database=${database_name} -format=json  | jq .results[0].series[0].values[0][1] | tee /dev/stderr | grep 116
