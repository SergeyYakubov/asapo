#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=11111111
beamtime_id2=22222222
data_source=python
beamline=p07
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}
receiver_folder2=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id2}
token=$BLP07_W_TOKEN

Cleanup() {
	echo cleanup
	rm -rf ${receiver_root_folder}
  echo "db.dropDatabase()" | mongo ${beamtime_id}_${data_source} >/dev/null
  echo "db.dropDatabase()" | mongo ${beamtime_id2}_${data_source} >/dev/null
  rm -rf $beamline_dir
}

export PYTHONPATH=$2:${PYTHONPATH}

mkdir -p ${receiver_folder} ${receiver_folder2}

sleep 1

echo test > file1

beamline_dir='/tmp/asapo/beamline/p07/current/'
mkdir -p $beamline_dir
cp beamtime-metadata-11111111.json $beamline_dir

$1 $3 $beamline $token $data_source "127.0.0.1:8400" ${beamline_dir}"beamtime-metadata-11111111.json" > out || cat out
cat out
cat out | grep "successfuly sent" | wc -l | tee /dev/stderr | grep 3
cat out | grep "reauthorization\|Broken" | wc -l | tee /dev/stderr | grep 1
cat out | grep "duplicated" | wc -l | tee /dev/stderr | grep 2

