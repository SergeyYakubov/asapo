#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3
network_type=$4

beamtime_id=asapo_test

stream1=s1
stream2=s2

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
    echo "db.dropDatabase()" | mongo ${beamtime_id}_${stream1}
    echo "db.dropDatabase()" | mongo ${beamtime_id}_${stream2}
}

echo "db.${beamtime_id}_${stream1}.insert({dummy:1})" | mongo ${beamtime_id}_${stream1}
echo "db.${beamtime_id}_${stream2}.insert({dummy:1})" | mongo ${beamtime_id}_${stream2}

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_tcp.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 5

token=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -types read $beamtime_id`

echo "Start producers"
mkdir -p ${receiver_folder}
$producer_bin localhost:8400 ${beamtime_id}%${stream1} 100 1000 4 0 100 &
$producer_bin localhost:8400 ${beamtime_id}%${stream2} 100 900 4 0 100 &

echo "Start consumers in $network_type mode"
$consumer_bin ${proxy_address} ${receiver_folder} ${beamtime_id}%${stream1} 2 $token 10000 0 | tee /dev/stderr consumer_1.out
$consumer_bin ${proxy_address} ${receiver_folder} ${beamtime_id}%${stream2} 2 $token 10000 0 | tee /dev/stderr consumer_2.out

grep "from memory buffer: 1000" consumer_1.out
grep -i "Using connection type: $network_type" consumer_1.out

grep "from memory buffer: 900" consumer_2.out
grep -i "Using connection type: $network_type" consumer_2.out
