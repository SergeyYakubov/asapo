#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
stream=$1
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

Cleanup() {
	echo cleanup
	rm -rf ${receiver_root_folder}
    nomad stop receiver >/dev/null
    nomad stop discovery >/dev/null
    nomad stop authorizer >/dev/null
    nomad stop nginx >/dev/null
    echo "db.dropDatabase()" | mongo ${beamtime_id}_${stream} >/dev/null
}

export PYTHONPATH=$2:${PYTHONPATH}

echo "db.${beamtime_id}_${stream}.insert({dummy:1})" | mongo ${beamtime_id}_${stream} >/dev/null

nomad run authorizer.nmd >/dev/null
nomad run nginx.nmd >/dev/null
nomad run receiver.nmd >/dev/null
nomad run discovery.nmd >/dev/null

mkdir -p ${receiver_folder}

echo test > file1

sleep 1

$1 $3 $stream $beamtime_id  "127.0.0.1:8400" > out
cat out
cat out | grep "successfuly sent" | wc -l | grep 7