#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$3 token -secret broker_secret.key $beamtime_id`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
#    kill $producerid
    echo "db.dropDatabase()" | mongo ${beamtime_id}
    influx -execute "drop database ${monitor_database_name}"
}

influx -execute "create database ${monitor_database_name}"
echo "db.${beamtime_id}.insert({dummy:1})" | mongo ${beamtime_id}

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

#producer
mkdir -p ${receiver_folder}
$1 localhost:8400 ${beamtime_id} 100 1000 4 0 100 &
#producerid=`echo $!`


$2 ${proxy_address} ${beamtime_id} 2 $token 1000 1 | grep "Processed 1000 file(s)"
