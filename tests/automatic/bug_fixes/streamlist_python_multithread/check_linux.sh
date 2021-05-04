#!/usr/bin/env bash
set -e

trap Cleanup EXIT

endpoint=127.0.0.1:8400
beamtime_id=asapo_test
token=$ASAPO_TEST_RW_TOKEN

Cleanup() {
    echo cleanup
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop broker
    nomad stop discovery
    nomad stop authorizer
    nomad stop receiver
    echo "db.dropDatabase()" | mongo ${beamtime_id}_source_1
    echo "db.dropDatabase()" | mongo ${beamtime_id}_source_2
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd
nomad run receiver_tcp.nmd
nomad run authorizer.nmd

sleep 1

export PYTHONPATH=$1:$2:${PYTHONPATH}
export Python3_EXECUTABLE=$3

$Python3_EXECUTABLE magic_producer.py $endpoint $beamtime_id $token > out
cat out
cat out | grep "5 : number of streams source_1:  5"
cat out | grep "5 : number of streams source_2:  5"
