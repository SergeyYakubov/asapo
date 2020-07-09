#!/usr/bin/env bash

set -e

trap Cleanup EXIT

file_transfer_folder=`pwd`/asap3/petra3/gpfs/p01/2019/data/aaa


Cleanup() {
	echo cleanup
  nomad stop authorizer
  nomad stop file_transfer
  rm -rf $file_transfer_folder aaa big_file
}

nomad run authorizer.nmd
nomad run file_transfer.nmd

sleep 1

mkdir -p $file_transfer_folder

token=bnCXpOdBV90wU1zybEw1duQNSORuwaKz6oDHqmL35p0= #token for aaa
folder_token=`curl --silent --data "{\"Folder\":\"$file_transfer_folder\",\"BeamtimeId\":\"aaa\",\"Token\":\"$token\"}" 127.0.0.1:5007/folder`
echo $folder_token


dd if=/dev/urandom of=$file_transfer_folder/aaa bs=1 count=100000

curl -o aaa --silent -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"aaa\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/transfer --stderr - | tee /dev/stderr

curl -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"aaa\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/transfer?sizeonly=true --stderr - | tee /dev/stderr | grep 100000


diff -q aaa $file_transfer_folder/aaa

dd if=/dev/zero of=$file_transfer_folder/big_file bs=1 count=0 seek=5368709120

curl -vvv -o big_file -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"big_file\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/transfer --stderr -  | tee /dev/stderr

ls -ln big_file | awk '{ print $5 }' | tee /dev/stderr | grep 5368709120



