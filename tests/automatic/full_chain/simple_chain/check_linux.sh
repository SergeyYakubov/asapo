#!/usr/bin/env bash

set -e

trap Cleanup EXIT

broker_database_name=test_run
monitor_database_name=db_test
broker_address=127.0.0.1:5005

Cleanup() {
	echo cleanup
	rm -rf files
    kill -9 $receiverid
    kill -9 $brokerid
    #kill -9 $producerrid
    echo "db.dropDatabase()" | mongo ${broker_database_name}
    influx -execute "drop database ${monitor_database_name}"
}

influx -execute "create database ${monitor_database_name}"


#receiver
$2 receiver.json &
sleep 0.3
receiverid=`echo $!`

#broker
$3 -config broker.json &
sleep 0.3
brokerid=`echo $!`


#producer
mkdir files
$1 localhost:4200 100 100 &
#producerrid=`echo $!`
sleep 0.1

$4 ${broker_address} ${broker_database_name} 2 | grep "Processed 100 file(s)"
