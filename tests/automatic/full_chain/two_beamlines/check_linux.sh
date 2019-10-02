#!/usr/bin/env bash

set -e

trap Cleanup EXIT

stream=detector

beamtime_id1=asapo_test1
token1=`$3 token -secret auth_secret.key $beamtime_id1`

beamtime_id2=asapo_test2
token2=`$3 token -secret auth_secret.key $beamtime_id2`

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
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
#    kill $producerid
    echo "db.dropDatabase()" | mongo ${beamtime_id1}_${stream}
    echo "db.dropDatabase()" | mongo ${beamtime_id2}_${stream}
    influx -execute "drop database ${monitor_database_name}"
}

echo "db.${beamtime_id1}_${stream}.insert({dummy:1})" | mongo ${beamtime_id1}_${stream}
echo "db.${beamtime_id2}_${stream}.insert({dummy:1})" | mongo ${beamtime_id2}_${stream}

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

#consumers
$2 ${proxy_address} ${receiver_folder1} ${beamtime_id1} 2 $token1 10000 0  | tee /dev/stderr | grep "Processed 1000 file(s)"
$2 ${proxy_address} ${receiver_folder2} ${beamtime_id2} 2 $token2 10000 0 | tee /dev/stderr | grep "Processed 900 file(s)"
