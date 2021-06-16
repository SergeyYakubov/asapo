#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
data_source=python
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

export PYTHONPATH=$2:${PYTHONPATH}

mkdir -p ${receiver_folder}

echo test > file1

$1 $3 $data_source $beamtime_id  "127.0.0.1:8400" &> out || cat out
cat out
echo count successfully send, expect 17
cat out | grep "successfuly sent" | wc -l | tee /dev/stderr | grep 17
echo count wrong input, expect 11
cat out | grep "wrong input" | wc -l | tee /dev/stderr | grep 11

echo count wrong json, expect 2
cat out | grep "JSON parse error" | wc -l | tee /dev/stderr | grep 2
echo count same id, expect 4
cat out | grep "already have record with same id" | wc -l | tee /dev/stderr | grep 4
echo count duplicates, expect 6
cat out | grep "duplicate" | wc -l | tee /dev/stderr | grep 6
echo count data in callback, expect 6
cat out | grep "'data':" | wc -l  | tee /dev/stderr | grep 6
echo check found local io error
cat out | grep "local i/o error"
cat out | grep "Finished successfully"
