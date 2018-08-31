#!/usr/bin/env bash

database_name=test_run

set -e

trap Cleanup EXIT

Cleanup() {
rm -rf files
}

mkdir files

$@ files beamtime_id 11 4 4 1 10 2>&1 | grep Rate

ls -ln files/0 | awk '{ print $5 }'| grep 11000
ls -ln files/1 | awk '{ print $5 }'| grep 11000
ls -ln files/2 | awk '{ print $5 }'| grep 11000
ls -ln files/3 | awk '{ print $5 }'| grep 11000
