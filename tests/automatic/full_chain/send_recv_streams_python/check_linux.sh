#!/usr/bin/env bash

source_path=.
beamtime_id=asapo_test
stream_in=detector

indatabase_name=${beamtime_id}_${stream_in}
token=$ASAPO_TEST_RW_TOKEN

beamline=test

set -e
set -o pipefail

trap Cleanup EXIT

Cleanup() {
  set +e
  set +o pipefail
	echo "db.dropDatabase()" | mongo ${indatabase_name}
}


export PYTHONPATH=$2:$3:${PYTHONPATH}


$1 $4 127.0.0.1:8400 $beamtime_id $token | tee out
