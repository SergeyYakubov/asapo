#!/usr/bin/env bash

database_name=test_detector

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
}

echo 'db.meta.insert({"_id":"bt","data":"test_bt"})' | mongo ${database_name}
echo 'db.meta.insert({"_id":"st_test","data":"test_st"})' | mongo ${database_name}

token=$BT_TEST_TOKEN

broker=`curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.3`
echo found broker at $broker


curl -v  --silent $broker/v0.2/beamtime/test/detector/default/0/meta/0?token=$token --stderr - | tee /dev/stderr | grep '"data":"test_bt"'
curl -v  --silent $broker/v0.2/beamtime/test/detector/test/0/meta/1?token=$token --stderr - | tee /dev/stderr | grep '"data":"test_st"'
curl -v  --silent $broker/v0.2/beamtime/test/detector/default/0/meta/1?token=$token --stderr - | tee /dev/stderr | grep 'no documents'

