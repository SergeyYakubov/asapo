#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$2 token -secret broker_secret.key $beamtime_id`

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
    rm -rf out
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -execute "drop database ${monitor_database_name}"
}

influx -execute "create database ${monitor_database_name}"
echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo ${beamtime_id}_detector

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

#producer
mkdir -p ${receiver_folder}
$1 localhost:8400 ${beamtime_id} 100 100 1 0 100

export PYTHONPATH=$4:${PYTHONPATH}


python $3/get_user_meta.py $proxy_address $receiver_folder $beamtime_id $token new > out
cat out
cat out | grep "found images: 100"
cat out | grep "test100"
