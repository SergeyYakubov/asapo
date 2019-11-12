#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
beamtime_id=asapo_test
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

Cleanup() {
	echo cleanup
	rm -rf ${receiver_root_folder}
    nomad stop receiver
    nomad stop discovery
    nomad stop authorizer
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -execute "drop database ${database_name}"
}

# create db before consumer starts reading it. todo: git rid of it
echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo ${beamtime_id}_detector

nomad run authorizer.nmd
nomad run nginx.nmd
nomad run receiver.nmd
nomad run discovery.nmd

mkdir -p ${receiver_folder}

sleep 1

$1 localhost:8400 ${beamtime_id} 100 1 1  0 30

ls -ln ${receiver_folder}/1 | awk '{ print $5 }'| grep 100000

$1 localhost:8400 wrong_beamtime_id 100 1 1 0 1 2>&1 | tee /dev/stderr | grep "authorization"
