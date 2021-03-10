#!/usr/bin/env bash

set -e

trap Cleanup EXIT

producer_bin=$1
consumer_bin=$2
asapo_tool_bin=$3
network_type=$7

beamtime_id=asapo_test

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}
mkdir -p ${receiver_folder}

Cleanup() {
    echo cleanup
    rm -rf ${receiver_folder}
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -execute "drop database ${monitor_database_name}"
}

#sed -i 's/info/debug/g' broker.json.tpl

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run receiver_tcp.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

token=`$asapo_tool_bin token -endpoint http://localhost:8400/asapo-authorizer -secret admin_token.key -type read $beamtime_id`

echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo  ${beamtime_id}_detector

echo "Start producer"
$producer_bin localhost:8400 ${beamtime_id} 100 $5 4 0 100 &
#producerid=`echo $!`

echo "Start consumer in $network_type mode"
$consumer_bin ${proxy_address} dummy_path ${beamtime_id} 2 $token 30000 1 &> output.txt &

sleep 1

nomad stop $4
nomad stop authorizer
nomad stop discovery
nomad stop nginx
nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
nomad stop receiver

nomad run nginx.nmd
nomad run authorizer.nmd
nomad run discovery.nmd
nomad run receiver_$7.nmd

if [[ "$4" == "receiver" ]]; then
  nomad run $4_$7.nmd
else
  nomad run $4.nmd
fi

wait

cat output.txt
nfiles=`cat output.txt | grep "Processed" | awk   '{print $2;}'`
test  $nfiles -ge $6
rm output.txt
