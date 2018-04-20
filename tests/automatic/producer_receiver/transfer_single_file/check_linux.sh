#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=test_run

Cleanup() {
	echo cleanup
	rm -rf files
    kill $receiverid
    echo "db.dropDatabase()" | mongo ${database_name}
}

nohup $2 receiver.json &>/dev/null &
sleep 0.3
receiverid=`echo $!`

mkdir files

$1 localhost:4200 100 1

ls -ln files/0.bin | awk '{ print $5 }'| grep 102400
