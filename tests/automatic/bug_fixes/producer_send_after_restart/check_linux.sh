#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3

beamtime_id=asapo_test

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


mkdir -p /tmp/asapo/test_in/test1

Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    rm -rf /tmp/asapo/test_in
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
}

sleep 1
mkdir  /tmp/asapo/test_in/processed
#producer
mkdir -p ${receiver_folder}
$producer_bin test.json &> output &
producerid=`echo $!`

sleep 1

echo hello > /tmp/asapo/test_in/processed/file1
sleep 1
nomad stop receiver
sleep 1
nomad run receiver_tcp.nmd

echo hello > /tmp/asapo/test_in/processed/file1
sleep 1

kill -s INT $producerid
sleep 0.5
cat output
cat output | grep "Processed 2"

