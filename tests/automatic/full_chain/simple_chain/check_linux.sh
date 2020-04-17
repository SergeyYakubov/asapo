#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3
network_type=$4

beamtime_id=asapo_test
token=`$asapo_tool_bin token -secret auth_secret.key $beamtime_id`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


Cleanup() {
    echo cleanup
    rm -rf ${receiver_root_folder}
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    rm -rf out
 #   kill $producerid
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -execute "drop database ${monitor_database_name}"
}

echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo ${beamtime_id}_detector

echo "db.dropDatabase()" | mongo ${beamtime_id}_detector


nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_${network_type}.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

echo "Start producer"
mkdir -p ${receiver_folder}
$producer_bin localhost:8400 ${beamtime_id} 100 1000 4 0 100
#producerid=`echo $!`

echo "Start consumer in $network_type mode"
$consumer_bin ${proxy_address} $network_type ${receiver_folder} ${beamtime_id} 2 $token 5000 1 | tee out
cat out
cat out | grep "Processed 1000 file(s)"
