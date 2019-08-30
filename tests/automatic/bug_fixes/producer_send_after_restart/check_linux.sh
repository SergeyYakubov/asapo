#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

mkdir -p /tmp/asapo/test_in/test1/

Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad stop receiver
    nomad stop discovery
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
}

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver.nmd
nomad run discovery.nmd

sleep 1

#producer
mkdir -p ${receiver_folder}
$1 test.json &> output &
producerid=`echo $!`

sleep 1

echo hello > /tmp/asapo/test_in/test1/file1
sleep 1
nomad stop receiver
sleep 1
nomad run receiver.nmd

echo hello > /tmp/asapo/test_in/test1/file1
sleep 1

kill -s INT $producerid
sleep 0.5
cat output
cat output | grep "Processed 2"

