#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=asapo_test

monitor_database_name=db_test
proxy_address=127.0.0.1:8400

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}

mkdir -p /tmp/asapo/test_in/test1/

Cleanup() {
    echo cleanup
    kill $producerid
    rm -rf /tmp/asapo/test_in/test1
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
nomad run receiver.nmd
nomad run discovery.nmd

sleep 1

#producer
mkdir -p ${receiver_folder}
$1 test.json &
producerid=`echo $!`

sleep 1

echo hello > /tmp/asapo/test_in/test1/file1

sleep 5

usage=`top -b -n 1 | grep receiver | awk  '{print int($9)'}`
echo CPU usage: $usage
if [ -z "$usage" ]; then
exit 1
fi

if (( $usage > 50 )); then
    exit 1
fi
