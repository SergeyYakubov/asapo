#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test
token=`$3 token -secret auth_secret.key $beamtime_id`

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

mkdir -p /tmp/asapo/test_in/test1/
mkdir -p /tmp/asapo/test_in/test2/

Cleanup() {
    echo cleanup
    kill $producerid
    rm -rf /tmp/asapo/test_in/test1
    rm -rf /tmp/asapo/test_in/test2
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
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

echo hello > /tmp/asapo/test_in/test1/file1
echo hello > /tmp/asapo/test_in/test1/file2
echo hello > /tmp/asapo/test_in/test2/file2

sleep 100

#$2 ${proxy_address} ${receiver_folder} ${beamtime_id} 2 $token 1000 1 | tee /dev/stderr | grep "Processed 3 file(s)"

test ! -f /tmp/asapo/test_in/test1/file1
test ! -f /tmp/asapo/test_in/test1/file2
test ! -f /tmp/asapo/test_in/test2/file2
