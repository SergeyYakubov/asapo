#!/usr/bin/env bash

set -e

trap Cleanup EXIT

broker_database_name=test_run
monitor_database_name=db_test
broker_address=127.0.0.1:5005

receiver_folder=/tmp/asapo/receiver/files

Cleanup() {
    echo cleanup
    rm -rf ${receiver_folder}
    nomad stop nginx
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
#    kill $producerid
    echo "db.dropDatabase()" | mongo ${broker_database_name}
    influx -execute "drop database ${monitor_database_name}"
}

influx -execute "create database ${monitor_database_name}"
echo "db.${broker_database_name}.insert({dummy:1})" | mongo ${broker_database_name}

nomad run nginx.nmd
nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1

#producer
mkdir -p ${receiver_folder}
$1 localhost:8400 100 1000 4 0 &
#producerid=`echo $!`


$2 ${broker_address} ${broker_database_name} 2 | grep "Processed 1000 file(s)"
