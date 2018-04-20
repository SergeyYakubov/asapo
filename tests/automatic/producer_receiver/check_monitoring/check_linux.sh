#!/usr/bin/env bash

database_name=db_test
mongo_database_name=test_run

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	influx -execute "drop database ${database_name}"
    kill $receiverid
	rm -rf files
    echo "db.dropDatabase()" | mongo ${mongo_database_name}
}

influx -execute "create database ${database_name}"

nohup $2 receiver.json &>/dev/null &
sleep 0.3
receiverid=`echo $!`

mkdir files

$1 localhost:4200 100 112

sleep 1

influx -execute "select sum(n_requests) from statistics" -database=${database_name} -format=json | jq .results[0].series[0].values[0][1] | grep 112
