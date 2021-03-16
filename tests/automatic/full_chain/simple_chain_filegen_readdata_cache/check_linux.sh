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


mkdir -p /tmp/asapo/test_in/processed

Cleanup() {
    echo cleanup
    kill -9 $producerid
    rm -rf /tmp/asapo/test_in
    rm -rf ${receiver_folder}
    influx -execute "drop database ${monitor_database_name}"
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    rm out.txt
}

echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo ${beamtime_id}_detector

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_${network_type}.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

token=`$3 token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -type read $beamtime_id`

echo "Start producer"
mkdir -p ${receiver_folder}
$producer_bin test.json &
producerid=`echo $!`

sleep 1
mkdir  /tmp/asapo/test_in/processed/test1
mkdir  /tmp/asapo/test_in/processed/test2

echo -n hello1 > /tmp/asapo/test_in/processed/test1/file1
echo -n hello2 > /tmp/asapo/test_in/processed/test1/file2
echo -n hello3 > /tmp/asapo/test_in/processed/test2/file1

echo "Start consumer in $network_type mode"
$consumer_bin ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 1000 0 | tee out.txt
grep "Processed 3 file(s)" out.txt
grep "hello1" out.txt
grep "hello2" out.txt
grep "hello3" out.txt
grep -i "Using connection type: $network_type" out.txt

sleep 12

influx -execute "select sum(n_requests) from statistics where receiver_ds_tag !=''" -database=${monitor_database_name} -format=json  | jq .results[0].series[0].values[0][1] | tee /dev/stderr | grep 3
