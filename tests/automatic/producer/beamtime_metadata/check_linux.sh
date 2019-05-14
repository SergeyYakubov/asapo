#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
   rm -rf files
}

mkdir files

$@ files beamtime_id 1

cat files/beamtime_global.meta | grep hello
ls -ln files/beamtime_global.meta | awk '{ print $5 }'| grep 5
