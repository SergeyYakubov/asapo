#!/usr/bin/env bash

database_name=db_test

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
	echo cleanup
	influx -execute "drop database ${database_name}"
	kill -9 $brokerid
}

influx -execute "create database ${database_name}"

$@ -config settings.json &

sleep 0.3

brokerid=`echo $!`

for i in `seq 1 50`;
do
    curl --silent 127.0.0.1:5005/database/data/next >/dev/null 2>&1 &
done


sleep 2

influx -execute "select sum(rate) from RequestsRate" -database=${database_name} -format=json | jq .results[0].series[0].values[0][1] | grep 50
