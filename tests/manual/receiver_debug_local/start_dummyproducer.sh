#!/usr/bin/env bash

beamtime_id=asapo_test

nfiles=1000
timeout=100
fsize=10000
mode=10 #tcp & no file write
nthreads=32

exec=/home/yakubov/projects/asapo/cmake-build-debug/examples/producer/dummy-data-producer/dummy-data-producer


$exec localhost:8400 ${beamtime_id} $fsize $nfiles  $nthreads $mode  $timeout
