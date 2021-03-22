#!/usr/bin/env bash

database_name=db_test

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
	echo cleanup
	influx -execute "drop database ${database_name}"
	kill -9 $brokerid
  nomad stop nginx
  nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
  nomad stop authorizer
}

! influx -execute "drop database ${database_name}"


nomad run nginx.nmd
nomad run authorizer.nmd
sleep 1


token=$BT_DATA_TOKEN


$1 -config settings.json &

sleep 0.3

brokerid=`echo $!`

groupid=`curl -d '' --silent 127.0.0.1:5005/creategroup`


for i in `seq 1 50`;
do
    curl --silent 127.0.0.1:5005/database/data/source/stream/${groupid}/next?token=$token >/dev/null 2>&1 &
done


sleep 3

influx -execute "select sum(rate) from RequestsRate" -database=${database_name} -format=json | jq .results[0].series[0].values[0][1] | tee /dev/stderr | grep 51

