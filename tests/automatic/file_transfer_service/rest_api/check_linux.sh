#!/usr/bin/env bash

set -e

trap Cleanup EXIT

file_transfer_folder=/tmp/asapo/asap3/petra3/gpfs/p01/2019/data/aaa


Cleanup() {
	echo cleanup
  rm -rf $file_transfer_folder aaa aaa1 big_file
}

mkdir -p $file_transfer_folder

token=$BT_AAA_TOKEN

folder_token=`curl --silent --data "{\"Folder\":\"$file_transfer_folder\",\"BeamtimeId\":\"aaa\",\"Token\":\"$token\"}" 127.0.0.1:5007/v0.1/folder`
echo $folder_token


dd if=/dev/urandom of=$file_transfer_folder/aaa bs=1 count=100000

curl -o aaa --silent -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"aaa\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/v0.1/transfer --stderr - | tee /dev/stderr

curl -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"aaa\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/v0.1/transfer?sizeonly=true --stderr - | tee /dev/stderr | grep 100000

diff -q aaa $file_transfer_folder/aaa

# auto folder
folder_token_auto=`curl --silent --data "{\"Folder\":\"auto\",\"BeamtimeId\":\"aaa\",\"Token\":\"$token\"}" 127.0.0.1:5007/v0.2/folder`
echo $folder_token_auto
curl -o aaa1 --silent -H "Authorization: Bearer ${folder_token_auto}" --data "{\"FileName\":\"aaa\",\"Token\":\"$folder_token_auto\"}" 127.0.0.1:5008/v0.2/transfer --stderr - | tee /dev/stderr
diff -q aaa1 $file_transfer_folder/aaa

#auto folder, old protocol
curl --silent --data "{\"Folder\":\"auto\",\"BeamtimeId\":\"aaa\",\"Token\":\"$token\"}" 127.0.0.1:5007/v0.1/folder | grep "auto does not match"
curl --silent -H "Authorization: Bearer ${folder_token_auto}" --data "{\"FileName\":\"aaa\",\"Token\":\"$folder_token_auto\"}" 127.0.0.1:5008/v0.1/transfer --stderr - | tee /dev/stderr | grep forbidden

exit 0



chmod -r $file_transfer_folder/aaa
curl --silent -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"aaa\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/v0.1/transfer?sizeonly=true --stderr - | tee /dev/stderr | grep "does not exist"


dd if=/dev/zero of=$file_transfer_folder/big_file bs=1 count=0 seek=5368709120

curl -vvv -o big_file -H "Authorization: Bearer ${folder_token}" --data "{\"Folder\":\"$file_transfer_folder\",\"FileName\":\"big_file\",\"Token\":\"$folder_token\"}" 127.0.0.1:5008/v0.1/transfer --stderr -  | tee /dev/stderr

ls -ln big_file | awk '{ print $5 }' | tee /dev/stderr | grep 5368709120

