#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$3 token -secret auth_secret.key $beamtime_id`

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
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -execute "drop database ${monitor_database_name}"
}

sed -i 's/info/debug/g' broker.json.tpl

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo  ${beamtime_id}_detector



#producer
$1 localhost:8400 ${beamtime_id} 100 $5 4 0 100 &
#producerid=`echo $!`


#consumer
$2 ${proxy_address} dummy_path ${beamtime_id} 2 $token 30000 1 &> output.txt &

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