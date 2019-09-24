#!/usr/bin/env bash

source_path=.
beamtime_id=asapo_test
stream_in=detector
stream_out=stream
stream_out2=stream2

indatabase_name=${beamtime_id}_${stream_in}
outdatabase_name=${beamtime_id}_${stream_out}
outdatabase_name2=${beamtime_id}_${stream_out2}

token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
receiver_folder=${receiver_root_folder}/${beamline}/${beamtime_id}


set -e

trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad stop discovery
    nomad stop broker
    nomad stop receiver
    nomad stop authorizer
	echo "db.dropDatabase()" | mongo ${indatabase_name}
	echo "db.dropDatabase()" | mongo ${outdatabase_name}
	rm -rf file1 file2 file3
    rm -rf ${receiver_root_folder}
    rm -rf out out2

}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd
nomad run receiver.nmd
nomad run authorizer.nmd

mkdir -p $receiver_folder

echo hello1 > file1
echo hello2 > file2
echo hello3 > file3

for i in `seq 1 3`;
do
	echo 'db.data.insert({"_id":'$i',"size":6,"name":"'file$i'","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}})' | mongo ${indatabase_name}
done

sleep 1

$1 127.0.0.1:8400 $source_path $beamtime_id $stream_in $stream_out $token 2 1000 1  > out
cat out
cat out | grep "Processed 3 file(s)"
cat out | grep "Sent 3 file(s)"

echo "db.data.find({"_id":1})" | mongo ${outdatabase_name} | tee /dev/stderr | grep file1_${stream_out}

cat ${receiver_folder}/file1_${stream_out} | grep hello1
cat ${receiver_folder}/file2_${stream_out} | grep hello2
cat ${receiver_folder}/file3_${stream_out} | grep hello3

$1 127.0.0.1:8400 $source_path $beamtime_id $stream_in $stream_out2 $token 2 1000 0  > out2
cat out2
test ! -f ${receiver_folder}/file1_${stream_out2}
echo "db.data.find({"_id":1})" | mongo ${outdatabase_name2} | tee /dev/stderr | grep ./file1