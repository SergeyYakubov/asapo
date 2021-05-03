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
    nomad stop receiver >/dev/null
    nomad stop discovery >/dev/null
    nomad stop authorizer >/dev/null
    nomad stop nginx >/dev/null
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill > /dev/null
    echo "db.dropDatabase()" | mongo ${beamtime_id}_${data_source} >/dev/null
}

export PYTHONPATH=$2:${PYTHONPATH}

echo "db.${beamtime_id}_${data_source}.insert({dummy:1})" | mongo ${beamtime_id}_${data_source}  >/dev/null

nomad run authorizer.nmd >/dev/null
nomad run nginx.nmd >/dev/null
nomad run receiver_tcp.nmd >/dev/null
nomad run discovery.nmd >/dev/null

mkdir -p ${receiver_folder}

echo test > file1

sleep 10

$1 $3 $data_source $beamtime_id  "127.0.0.1:8400" &> out || cat out
cat out
echo count successfully send, expect 15
cat out | grep "successfuly sent" | wc -l | tee /dev/stderr | grep 15
echo count same id, expect 4
cat out | grep "already have record with same id" | wc -l | tee /dev/stderr | grep 4
echo count duplicates, expect 6
cat out | grep "duplicate" | wc -l | tee /dev/stderr | grep 6
echo count data in callback, expect 3
cat out | grep "'data':" | wc -l  | tee /dev/stderr | grep 3
echo check found local io error
cat out | grep "local i/o error"
cat out | grep "Finished successfully"
