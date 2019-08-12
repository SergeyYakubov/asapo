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

token=`$2 token -secret broker_secret.key data`


$1 -config settings.json &

sleep 0.3

brokerid=`echo $!`

groupid=`curl -d '' --silent 127.0.0.1:5005/creategroup`


for i in `seq 1 50`;
do
    curl --silent 127.0.0.1:5005/database/data/stream/${groupid}/next?token=$token >/dev/null 2>&1 &
done


sleep 3

influx -execute "select sum(rate) from RequestsRate" -database=${database_name} -format=json | jq .results[0].series[0].values[0][1] | tee /dev/stderr | grep 51

