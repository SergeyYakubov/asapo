#!/usr/bin/env bash

source_path=.
beamtime_id=asapo_test
stream_in=detector

indatabase_name=${beamtime_id}_${stream_in}

#asapo_test read token
token=$ASAPO_TEST_RW_TOKEN

beamline=test

set -e

trap Cleanup EXIT

network_type=$2

Cleanup() {
    set +e
	echo "db.dropDatabase()" | mongo ${indatabase_name}
}

$1 127.0.0.1:8400 $beamtime_id $token | tee out

