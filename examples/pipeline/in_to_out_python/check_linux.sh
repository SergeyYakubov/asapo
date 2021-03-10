#!/usr/bin/env bash

source_path=.
beamtime_id=asapo_test
data_source_in=detector
data_source_out=data_source

timeout=15
timeout_producer=25
nthreads=4

indatabase_name=${beamtime_id}_${data_source_in}
outdatabase_name=${beamtime_id}_${data_source_out}

#asapo_test read token
token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTRhcDQzaXB0M2E0bmNpMDkwMCIsInN1YiI6ImJ0X2FzYXBvX3Rlc3QiLCJFeHRyYUNsYWltcyI6eyJBY2Nlc3NUeXBlIjoicmVhZCJ9fQ.X5Up3PBd81i4X7wUBXGkIrLEVSL-WO9kijDtzOqasgg

beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}



set -e

trap Cleanup EXIT

Cleanup() {
    set +e
    nomad stop nginx
    nomad run nginx_kill.nmd  && nomad stop -yes -purge nginx_kill
    nomad stop discovery
    nomad stop broker
    nomad stop receiver
    nomad stop authorizer
    echo "db.dropDatabase()" | mongo ${indatabase_name}
  	echo "db.dropDatabase()" | mongo ${outdatabase_name}
  	rm -rf processed
    rm -rf ${receiver_root_folder}
    rm -rf out

}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd
nomad run receiver_tcp.nmd
nomad run authorizer.nmd

mkdir -p $receiver_folder

mkdir processed
echo hello1 > processed/file1
echo hello2 > processed/file2
echo hello3 > processed/file3

for i in `seq 1 3`;
do
	echo 'db.data_default.insert({"_id":'$i',"size":6,"name":"'processed/file$i'","timestamp":1,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}})' | mongo ${indatabase_name}
done

sleep 1

export PYTHONPATH=$2:$3:${PYTHONPATH}


$1 $4 127.0.0.1:8400 $source_path $beamtime_id $data_source_in $data_source_out $token $timeout $timeout_producer $nthreads 1  > out
cat out
cat out | grep "Processed 3 file(s)"
cat out | grep "Sent 3 file(s)"

echo "db.data_default.find({"_id":1})" | mongo ${outdatabase_name} | tee /dev/stderr | grep "file1_${data_source_out}"

cat ${receiver_folder}/processed/file1_${data_source_out} | grep hello1
cat ${receiver_folder}/processed/file2_${data_source_out} | grep hello2
cat ${receiver_folder}/processed/file3_${data_source_out} | grep hello3
