#!/usr/bin/env bash

beamtime_id=asapo_test

token=
timeout=100
metaonly=0
nthreads=4
token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=

exec=/home/yakubov/projects/asapo/cmake-build-debug/examples/consumer/getnext_broker/getnext_broker

$exec localhost:8400 /tmp/asapo/receiver/files/test/asapo_test asapo_test $nthreads $token $timeout $metaonly
