#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
mongo_database_name=test_run

Cleanup() {
	echo cleanup
	rm -rf files
    nomad stop receiver
    nomad stop discovery
    echo "db.dropDatabase()" | mongo ${mongo_database_name}
    influx -execute "drop database ${database_name}"
}

influx -execute "create database ${database_name}"
echo "db.${mongo_database_name}.insert({dummy:1})" | mongo ${mongo_database_name}

nomad run receiver.nmd
nomad run discovery.nmd

mkdir files

$1 localhost:5006 100 1 1  0


ls -ln files/1.bin | awk '{ print $5 }'| grep 102400
