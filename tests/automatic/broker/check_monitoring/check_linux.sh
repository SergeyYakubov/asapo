#!/usr/bin/env bash

database_name=db_test

set -e

trap Cleanup EXIT

Cleanup() {
  set +e
	echo cleanup
	influx -execute "drop database ${database_name}"
}

! influx -execute "drop database ${database_name}"


token=$BT_DATA_TOKEN

broker=`curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.3`
echo found broker at $broker

groupid=`curl -d '' --silent $broker/v0.2/creategroup`


for i in `seq 1 50`;
do
    curl --silent $broker/v0.2/beamtime/data/source/stream/${groupid}/next?token=$token >/dev/null 2>&1 &
done

sleep 12

influx -execute "select sum(rate) from RequestsRate" -database=${database_name} -format=json | jq .results[0].series[0].values[0][1]

