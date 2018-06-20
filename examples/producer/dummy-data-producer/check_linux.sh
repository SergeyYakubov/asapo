#!/usr/bin/env bash

database_name=test_run

set -e

trap Cleanup EXIT

Cleanup() {
rm -rf files
}

mkdir files

$@ files beamtime_id 11 4 4 1 10 2>&1 | grep Rate


ls -ln files/0.bin | awk '{ print $5 }'| grep 11264
ls -ln files/1.bin | awk '{ print $5 }'| grep 11264
ls -ln files/2.bin | awk '{ print $5 }'| grep 11264
ls -ln files/3.bin | awk '{ print $5 }'| grep 11264
