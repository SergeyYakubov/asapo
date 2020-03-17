#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=aaa
file_transfer_folder=`pwd`/asap3/petra3/gpfs/p01/2019/data/$beamtime_id

Cleanup() {
  echo cleanup
  nomad stop authorizer
  nomad stop file_transfer
  rm -rf $file_transfer_folder bbb
}

nomad run authorizer.nmd
nomad run file_transfer.nmd
sleep 1

mkdir -p $file_transfer_folder
echo -n hello > $file_transfer_folder/aaa

$1  127.0.0.1:5007 127.0.0.1:5008 $file_transfer_folder aaa
cat bbb | tee /dev/stderr | grep hello


