#!/usr/bin/env bash

set -e

trap Cleanup EXIT

beamtime_id=aaa
file_transfer_folder=/tmp/asapo/asap3/petra3/gpfs/p01/2019/data/$beamtime_id

Cleanup() {
  echo cleanup
  rm -rf $file_transfer_folder bbb random
}

mkdir -p $file_transfer_folder
echo -n hello > $file_transfer_folder/aaa

dd if=/dev/urandom of=$file_transfer_folder/random bs=1 count=100000

$1  127.0.0.1:5007 127.0.0.1:5008 $file_transfer_folder
cat bbb | tee /dev/stderr | grep hello
diff -q random $file_transfer_folder/random


