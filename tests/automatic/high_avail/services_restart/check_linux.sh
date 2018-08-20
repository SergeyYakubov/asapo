#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$3 token -secret broker_secret.key $beamtime_id`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test

Cleanup() {
    echo cleanup
    nomad stop nginx
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}
    influx -execute "drop database ${monitor_database_name}"
}

influx -execute "create database ${monitor_database_name}"

sed -i 's/"WriteToDisk":true/"WriteToDisk":false/g' receiver.json.tpl
sed -i 's/info/debug/g' broker.json.tpl

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

echo "db.${beamtime_id}.insert({dummy:1})" | mongo  ${beamtime_id}



#producer
$1 localhost:8400 ${beamtime_id} 100 $5 4 0 100 &
#producerid=`echo $!`


#worker
$2 ${proxy_address} ${beamtime_id} 2 $token 20000 &> output.txt &

sleep 1

nomad stop $4
nomad stop authorizer
nomad stop discovery
nomad stop nginx
nomad stop receiver

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run discovery.nmd
nomad run receiver.nmd

nomad run $4.nmd

wait

cat output.txt
nfiles=`cat output.txt | grep "Processed" | awk   '{print $2;}'`
test  $nfiles -ge $6
rm output.txt