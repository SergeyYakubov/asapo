#!/usr/bin/env bash

source_path=.
beamtime_id=asapo_test
stream_in=detector

indatabase_name=${beamtime_id}_${stream_in}
token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=

beamline=test

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
}

nomad run nginx.nmd
nomad run discovery.nmd
nomad run broker.nmd
nomad run receiver.nmd
nomad run authorizer.nmd


export PYTHONPATH=$2:$3:${PYTHONPATH}


$1 $4 127.0.0.1:8400 $beamtime_id $token