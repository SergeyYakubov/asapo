#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id1=asapo_test1
token1=`$3 token -secret broker_secret.key $beamtime_id1`

beamtime_id2=asapo_test2
token2=`$3 token -secret broker_secret.key $beamtime_id2`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline1=test1
beamline2=test2
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder1=${receiver_root_folder}/${beamline1}/${beamtime_id1}
receiver_folder2=${receiver_root_folder}/${beamline2}/${beamtime_id2}

Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
#    kill $producerid
    echo "db.dropDatabase()" | mongo ${beamtime_id1}
    echo "db.dropDatabase()" | mongo ${beamtime_id2}
    influx -execute "drop database ${monitor_database_name}"
}

influx -execute "create database ${monitor_database_name}"
echo "db.${beamtime_id1}.insert({dummy:1})" | mongo ${beamtime_id1}
echo "db.${beamtime_id2}.insert({dummy:1})" | mongo ${beamtime_id2}

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 3

#producer
mkdir -p ${receiver_folder1}
mkdir -p ${receiver_folder2}
$1 localhost:8400 ${beamtime_id1} 100 1000 4 0 100 &
$1 localhost:8400 ${beamtime_id2} 100 900 4 0 100 &
#producerid=`echo $!`


$2 ${proxy_address} ${beamtime_id1} 2 $token1 5000 | tee /dev/stderr | grep "Processed 1000 file(s)"
$2 ${proxy_address} ${beamtime_id2} 2 $token2 5000 | tee /dev/stderr | grep "Processed 900 file(s)"
