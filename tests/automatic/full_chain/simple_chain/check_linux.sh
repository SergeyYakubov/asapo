#!/usr/bin/env bash

set -e

trap Cleanup EXIT

broker_database_name=test_run
monitor_database_name=db_test
broker_address=127.0.0.1:5005

Cleanup() {
	echo cleanup
	rm -rf files
    nomad stop receiver
    nomad stop discovery
    nomad stop broker
    echo "db.dropDatabase()" | mongo ${broker_database_name}
    influx -execute "drop database ${monitor_database_name}"
}

influx -execute "create database ${monitor_database_name}"

nomad run receiver.nmd
nomad run discovery.nmd
nomad run broker.nmd

sleep 1


#producer
mkdir files
$1 localhost:5006 100 100 4 0 &

#producerrid=`echo $!`
sleep 0.1

$4 ${broker_address} ${broker_database_name} 2 | grep "Processed 100 file(s)"
