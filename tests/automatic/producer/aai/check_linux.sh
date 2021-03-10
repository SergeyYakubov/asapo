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
token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTRhbHRyaXB0MzltZTRqcXB0ZyIsInN1YiI6ImJsX3AwNyIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJ3cml0ZSJ9fQ._yy0E42cOGMv81GDj3WKZJlF8mBmjKtHNDPnN5NTxvk # write token for bl_p07

Cleanup() {
	echo cleanup
	rm -rf ${receiver_root_folder}
    nomad stop receiver >/dev/null
    nomad stop discovery >/dev/null
    nomad stop authorizer >/dev/null
    nomad stop nginx >/dev/null
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill > /dev/null
    echo "db.dropDatabase()" | mongo ${beamtime_id}_${data_source} >/dev/null
    echo "db.dropDatabase()" | mongo ${beamtime_id2}_${data_source} >/dev/null
}

export PYTHONPATH=$2:${PYTHONPATH}

nomad run authorizer.nmd >/dev/null
nomad run nginx.nmd >/dev/null
nomad run receiver_tcp.nmd >/dev/null
nomad run discovery.nmd >/dev/null

mkdir -p ${receiver_folder} ${receiver_folder2}

sleep 1

echo test > file1


$1 $3 $beamline $token $data_source "127.0.0.1:8400" > out || cat out
cat out
cat out | grep "successfuly sent" | wc -l | grep 3
cat out | grep "reauthorization\|Broken" | wc -l | grep 1
cat out | grep "duplicated" | wc -l | grep 2

