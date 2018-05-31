#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
mongo_database_name=test_run
receiver_folder=/tmp/asapo/receiver/files

Cleanup() {
	echo cleanup
	rm -rf ${receiver_folder}
    nomad stop receiver
    nomad stop discovery
    echo "db.dropDatabase()" | mongo ${mongo_database_name}
    influx -execute "drop database ${database_name}"
}

influx -execute "create database ${database_name}"
echo "db.${mongo_database_name}.insert({dummy:1})" | mongo ${mongo_database_name}

nomad run receiver.nmd
nomad run discovery.nmd

mkdir -p ${receiver_folder}

$1 localhost:5006 100 1 1  0


ls -ln ${receiver_folder}/1.bin | awk '{ print $5 }'| grep 102400
