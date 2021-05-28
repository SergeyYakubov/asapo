#!/usr/bin/env bash

source_path=.
beamtime_id=asapo_test
data_source_in=detector
data_source_out=data_source
data_source_out2=data_source2

indatabase_name=${beamtime_id}_${data_source_in}
outdatabase_name=${beamtime_id}_${data_source_out}
outdatabase_name2=${beamtime_id}_${data_source_out2}

#asapo_test read token
token=$ASAPO_TEST_RW_TOKEN

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
	  echo "db.dropDatabase()" | mongo ${indatabase_name}
	  echo "db.dropDatabase()" | mongo ${outdatabase_name}
    echo "db.dropDatabase()" | mongo ${outdatabase_name2}
	  rm -rf processed
    rm -rf ${receiver_root_folder}
    rm -rf out out2

}

mkdir -p $receiver_folder
mkdir processed
echo hello1 > processed/file1
echo hello2 > processed/file2
echo hello3 > processed/file3

for i in `seq 1 3`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":6,"name":"'processed/file$i'","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${indatabase_name}
done

sleep 1

$1 127.0.0.1:8400 $source_path $beamtime_id $data_source_in $data_source_out $token 2 1000 25000 1  > out
cat out
cat out | grep "Processed 3 file(s)"
cat out | grep "Sent 3 file(s)"

echo "db.data_default.find({"_id":1})" | mongo ${outdatabase_name} | tee /dev/stderr | grep file1_${data_source_out}

cat ${receiver_folder}/processed/file1_${data_source_out} | grep hello1
cat ${receiver_folder}/processed/file2_${data_source_out} | grep hello2
cat ${receiver_folder}/processed/file3_${data_source_out} | grep hello3

$1 127.0.0.1:8400 $source_path $beamtime_id $data_source_in $data_source_out2 $token 2 1000 25000 0  > out2
cat out2
test ! -f ${receiver_folder}/processed/file1_${data_source_out2}
echo "db.data_default.find({"_id":1})" | mongo ${outdatabase_name2} | tee /dev/stderr | grep processed/file1
