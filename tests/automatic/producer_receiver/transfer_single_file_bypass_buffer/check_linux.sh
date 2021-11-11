#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
beamtime_id=asapo_test
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


Cleanup() {

	echo cleanup
	rm -rf ${receiver_root_folder}
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -database ${database_name} -execute "drop series from statistics, RequestsRate"
}

mkdir -p ${receiver_folder}

$1 localhost:8400 ${beamtime_id} 60000 1 1  0 30

echo "db.data_default.find({"_id":1})" | mongo ${beamtime_id}_detector  > out
cat out
cat out | grep '"buf_id" : 0'
cat out | grep user_meta

ls -ln ${receiver_folder}/processed/1 | awk '{ print $5 }'| grep 60000000
