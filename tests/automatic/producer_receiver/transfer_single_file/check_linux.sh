#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
beamtime_id=asapo_test
receiver_folder=/tmp/asapo/receiver/files/${beamtime_id}

Cleanup() {
	echo cleanup
	rm -rf ${receiver_folder}
    nomad stop receiver
    nomad stop discovery
    nomad stop authorizer
    nomad stop nginx
    echo "db.dropDatabase()" | mongo ${beamtime_id}
    influx -execute "drop database ${database_name}"
}

influx -execute "create database ${database_name}"
# create db before worker starts reading it. todo: git rid of it
echo "db.${beamtime_id}.insert({dummy:1})" | mongo ${beamtime_id}

nomad run authorizer.nmd
nomad run nginx.nmd
nomad run receiver.nmd
nomad run discovery.nmd

mkdir -p ${receiver_folder}

$1 localhost:8400 ${beamtime_id} 100 1 1  0 30

ls -ln ${receiver_folder}/1.bin | awk '{ print $5 }'| grep 102400


$1 localhost:8400 wrong_beamtime_id 100 1 1 0 1 2>1 | grep "authorization failed"