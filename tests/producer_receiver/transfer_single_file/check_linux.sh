#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
    kill $receiverid
	rm -rf files
}

nohup $2 &>/dev/null &
sleep 0.3
receiverid=`echo $!`

$1 localhost:4200 100 1

du -b files/0.bin | cut -f1 | grep 100
