#!/usr/bin/env bash

database_name=db_test
beamtime_id=asapo_test
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	influx -execute "drop database ${database_name}"
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    rm -rf ${receiver_root_folder}
}

mkdir -p ${receiver_folder}

$1 localhost:8400 ${beamtime_id} 100 112 4  0 100

sleep 2

# should be 116 requests (112 data transfers +  4 authorizations)
influx -execute "select sum(n_requests) from statistics" -database=${database_name} -format=json | tee /dev/stderr  | jq .results[0].series[0].values[0][1] | tee /dev/stderr | grep 116
