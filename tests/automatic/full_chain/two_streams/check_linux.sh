#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$3 token -secret auth_secret.key $beamtime_id`

stream1=s1
stream2=s2

proxy_address=127.0.0.1:8400

beamline=test

receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}_${stream1}
    echo "db.dropDatabase()" | mongo ${beamtime_id}_${stream2}
}

echo "db.${beamtime_id}_${stream1}.insert({dummy:1})" | mongo ${beamtime_id}_${stream1}
echo "db.${beamtime_id}_${stream2}.insert({dummy:1})" | mongo ${beamtime_id}_${stream2}

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 3

#producer
mkdir -p ${receiver_folder}
$1 localhost:8400 ${beamtime_id}%${stream1} 100 1000 4 0 100 &
$1 localhost:8400 ${beamtime_id}%${stream2} 100 900 4 0 100 &


#consumers
$2 ${proxy_address} ${receiver_folder} ${beamtime_id}%${stream1} 2 $token 10000 0  | tee /dev/stderr | grep "Processed 1000 file(s)"
$2 ${proxy_address} ${receiver_folder} ${beamtime_id}%${stream2} 2 $token 10000 0 | tee /dev/stderr | grep "Processed 900 file(s)"
