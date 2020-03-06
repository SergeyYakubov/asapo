#!/usr/bin/env bash

set -e

trap Cleanup EXIT

file_transfer_folder=/tmp/asapo/file_transfer/files


Cleanup() {
	echo cleanup
  nomad stop authorizer
  nomad stop file_transfer
  rm -rf $file_transfer_folder aaa big_file
}

nomad run authorizer.nmd
nomad run file_transfer.nmd

sleep 1

token=bnCXpOdBV90wU1zybEw1duQNSORuwaKz6oDHqmL35p0= #token for aaa
folder_token=`curl --silent --data "{\"Folder\":\"$file_transfer_folder\",\"BeamtimeId\":\"aaa\",\"Token\":\"$token\"}" 127.0.0.1:5007/folder`

mkdir -p $file_transfer_folder
echo hello > $file_transfer_folder/aaa

curl -o aaa --silent -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"aaa\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/transfer --stderr - | tee /dev/stderr

cat aaa | grep hello

dd if=/dev/zero of=$file_transfer_folder/big_file bs=1 count=0 seek=5368709120

curl -vvv -o big_file -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"big_file\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/transfer --stderr -  | tee /dev/stderr

ls -ln big_file | awk '{ print $5 }' | tee /dev/stderr | grep 5368709120



