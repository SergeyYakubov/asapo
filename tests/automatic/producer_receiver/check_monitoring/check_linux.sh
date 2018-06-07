#!/usr/bin/env bash

database_name=db_test
mongo_database_name=test_run
receiver_folder=/tmp/asapo/receiver/files
set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	influx -execute "drop database ${database_name}"
    nomad stop receiver
    nomad stop discovery
    nomad stop nginx
    echo "db.dropDatabase()" | mongo ${mongo_database_name}
    rm -rf ${receiver_folder}
}

mkdir -p ${receiver_folder}

influx -execute "create database ${database_name}"

nomad run receiver.nmd
nomad run discovery.nmd
nomad run nginx.nmd

sleep 1

$1 localhost:8400 100 112 4  0

sleep 1

influx -execute "select sum(n_requests) from statistics" -database=${database_name} -format=json | jq .results[0].series[0].values[0][1] | grep 112
