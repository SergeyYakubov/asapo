#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
data_source=python
beamtime_id=asapo_test
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}
receiver_folder_online=${receiver_root_folder}/beamline/${beamline}/current

Cleanup() {
	echo cleanup
	rm -rf ${receiver_root_folder} ${receiver_folder_online}
  echo "db.dropDatabase()" | mongo ${beamtime_id}_${data_source}
  influx -database ${database_name} -execute "drop series from statistics, RequestsRate"
}

mkdir -p ${receiver_folder}
mkdir -p ${receiver_folder_online}


export PYTHONPATH=$2:${PYTHONPATH}

mkdir -p ${receiver_folder}

echo test > file1

$1 $3 $data_source $beamtime_id  "127.0.0.1:8400" &> out
cat out

ls -ln ${receiver_folder}/raw/python/file1 | awk '{ print $5 }'| grep 5
ls -ln ${receiver_folder_online}/raw/python/file1 | awk '{ print $5 }'| grep 5

