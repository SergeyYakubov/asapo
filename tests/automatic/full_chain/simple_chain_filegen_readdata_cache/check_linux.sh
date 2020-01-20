#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$3 token -secret auth_secret.key $beamtime_id`

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
    kill $producerid
    influx -execute "drop database ${monitor_database_name}"
    rm -rf /tmp/asapo/test_in/test1
    rm -rf /tmp/asapo/test_in/test2
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
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

#producer
mkdir -p ${receiver_folder}
$1 test.json &
producerid=`echo $!`

sleep 1

echo -n hello1 > /tmp/asapo/test_in/test1/file1
echo -n hello2 > /tmp/asapo/test_in/test1/file2
echo -n hello3 > /tmp/asapo/test_in/test2/file2

$2 ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 1000 0 > out.txt
cat out.txt
grep "Processed 3 file(s)" out.txt
grep "hello1" out.txt
grep "hello2" out.txt
grep "hello3" out.txt

sleep 12

influx -execute "select sum(n_requests) from statistics where receiver_ds_tag !=''" -database=${monitor_database_name} -format=json  | jq .results[0].series[0].values[0][1] | tee /dev/stderr | grep 3
