#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3

beamtime_id=asapo_test
token=`$asapo_tool_bin token -secret auth_secret.key $beamtime_id`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


mkdir -p /tmp/asapo/test_in/test1/
mkdir -p /tmp/asapo/test_in/test2/

Cleanup() {
    echo cleanup
    kill $producerid1
    kill $producerid2
    rm -rf /tmp/asapo/test_in/test1
    rm -rf /tmp/asapo/test_in/test2
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    rm -rf out
}

echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo ${beamtime_id}_detector

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_tcp.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

mkdir -p ${receiver_folder}

echo "Start producer 1"
$producer_bin test1.json &
producerid1=`echo $!`

echo "Start producer 2"
$producer_bin test2.json &
producerid2=`echo $!`

sleep 1

echo hello > /tmp/asapo/test_in/test1/file1
echo hello > /tmp/asapo/test_in/test1/file2
echo hello > /tmp/asapo/test_in/test2/file1
echo hello > /tmp/asapo/test_in/test2/file2

echo "Start consumer in metadata only mode"
$consumer_bin ${proxy_address} $network_type ${receiver_folder} ${beamtime_id} 2 $token 2000 1 1 | tee out
grep "Processed 2 dataset(s)" out
grep "with 4 file(s)" out
grep -i "Using connection type: No connection" out
