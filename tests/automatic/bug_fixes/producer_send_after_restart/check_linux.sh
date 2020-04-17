#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3
network_type=$4

beamtime_id=asapo_test

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


mkdir -p /tmp/asapo/test_in/test1/

Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
}

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_${network_type}.nmd
nomad run discovery.nmd

sleep 1

#producer
mkdir -p ${receiver_folder}
$producer_bin test.json &> output &
producerid=`echo $!`

sleep 1

echo hello > /tmp/asapo/test_in/test1/file1
sleep 1
nomad stop receiver
sleep 1
nomad run receiver_${network_type}.nmd

echo hello > /tmp/asapo/test_in/test1/file1
sleep 1

kill -s INT $producerid
sleep 0.5
cat output
cat output | grep "Processed 2"

