#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=test_run

Cleanup() {
	echo cleanup
	rm -rf files
    nomad stop receiver
    nomad stop discovery
    echo "db.dropDatabase()" | mongo ${database_name}
}

nomad run receiver.nmd
nomad run discovery.nmd

mkdir files

$1 localhost:5006 100 1 1  0


ls -ln files/1.bin | awk '{ print $5 }'| grep 102400
