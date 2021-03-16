#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3
network_type=$4

data_source=detector

beamtime_id1=asapo_test1

beamtime_id2=asapo_test2

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline1=test1
beamline2=test2
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder1=${receiver_root_folder}/${facility}/gpfs/${beamline1}/${year}/data/${beamtime_id1}
receiver_folder2=${receiver_root_folder}/${facility}/gpfs/${beamline2}/${year}/data/${beamtime_id2}



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
    echo "db.dropDatabase()" | mongo ${beamtime_id1}_${data_source}
    echo "db.dropDatabase()" | mongo ${beamtime_id2}_${data_source}
    influx -execute "drop database ${monitor_database_name}"
}

echo "db.${beamtime_id1}_${data_source}.insert({dummy:1})" | mongo ${beamtime_id1}_${data_source}
echo "db.${beamtime_id2}_${data_source}.insert({dummy:1})" | mongo ${beamtime_id2}_${data_source}

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_${network_type}.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 3

token1=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -type read $beamtime_id1`
token2=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -type read $beamtime_id2`

echo "Start producers"
mkdir -p ${receiver_folder1}
mkdir -p ${receiver_folder2}
$producer_bin localhost:8400 ${beamtime_id1} 100 1000 4 0 100 &
$producer_bin localhost:8400 ${beamtime_id2} 100 900 4 0 100 &
#producerid=`echo $!`

echo "Start consumers in $network_type mode"
$consumer_bin ${proxy_address} ${receiver_folder1} ${beamtime_id1} 2 $token1 12000 0 | tee /dev/stderr consumer_1.out
$consumer_bin ${proxy_address} ${receiver_folder2} ${beamtime_id2} 2 $token2 12000 0 | tee /dev/stderr consumer_2.out

grep "from memory buffer: 1000" consumer_1.out
grep -i "Using connection type: $network_type" consumer_1.out

grep "from memory buffer: 900" consumer_2.out
grep -i "Using connection type: $network_type" consumer_2.out
