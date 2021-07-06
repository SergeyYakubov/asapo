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
  influx -execute "drop database ${database_name}"
}

mkdir -p ${receiver_folder}

$1 localhost:8400 ${beamtime_id} 100 1 1  0 30

ls -ln ${receiver_folder}/processed/1 | awk '{ print $5 }'| grep 100000

$1 localhost:8400 wrong_beamtime_id 100 1 1 0 1 2>&1 | tee /dev/stderr | grep "authorization"
