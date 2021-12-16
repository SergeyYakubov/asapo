#!/usr/bin/env bash
set -e

trap Cleanup EXIT

Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
}



export PYTHONPATH=$2:${PYTHONPATH}
export Python3_EXECUTABLE=$3

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
beamtime_id=asapo_test
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}
endpoint=127.0.0.1:8400

mkdir -p ${receiver_folder}
echo ${receiver_folder}
$Python3_EXECUTABLE $1 $endpoint $beamtime_id


