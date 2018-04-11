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

mkdir files

$1 localhost:4200 100 1

ls -ln files/0.bin | awk '{ print $5 }'| grep 102400
