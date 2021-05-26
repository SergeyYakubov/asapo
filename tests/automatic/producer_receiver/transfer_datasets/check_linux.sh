#!/usr/bin/env bash

set -e

trap Cleanup EXIT

database_name=db_test
beamtime_id=asapo_test
beamline=test
receiver_root_folder=/tmp/asapo/receiver/files
facility=test_facility
year=2019
receiver_folder=${receiver_root_folder}/${facility}/gpfs/${beamline}/${year}/data/${beamtime_id}


Cleanup() {
	echo cleanup
	rm -rf ${receiver_root_folder}
    echo "db.dropDatabase()" | mongo ${beamtime_id}_detector
    influx -execute "drop database ${database_name}"
}

echo "db.dropDatabase()" | mongo ${beamtime_id}_detector


# create db before consumer starts reading it. todo: git rid of it
echo "db.${beamtime_id}_detector.insert({dummy:1})" | mongo ${beamtime_id}_detector

mkdir -p ${receiver_folder}

$1 localhost:8400 ${beamtime_id} 100 1 1 0 30 3

ls -ln ${receiver_folder}/processed/1_1 | awk '{ print $5 }'| grep 100000
ls -ln ${receiver_folder}/processed/1_2 | awk '{ print $5 }'| grep 100000
ls -ln ${receiver_folder}/processed/1_3 | awk '{ print $5 }'| grep 100000

echo 'db.data_default.find({"messages._id":{$gt:0}},{"messages.name":1})' | mongo asapo_test_detector | grep 1_1 | grep 1_2 | grep 1_3
