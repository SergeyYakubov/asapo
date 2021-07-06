#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
data_source=cpp
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


Cleanup() {
	echo cleanup
	rm -rf ${receiver_root_folder}
  echo "db.dropDatabase()" | mongo ${beamtime_id}_${data_source} >/dev/null
}

mkdir -p ${receiver_folder}

echo test > file1

$@ "127.0.0.1:8400" $data_source $beamtime_id
