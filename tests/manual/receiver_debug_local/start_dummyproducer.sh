#!/usr/bin/env bash

beamtime_id=asapo_test

nfiles=10
timeout=100
fsize=100
mode=0 #tcp
nthreads=4

exec=/home/yakubov/projects/asapo/cmake-build-debug/examples/producer/dummy-data-producer/dummy-data-producer


$exec localhost:8400 ${beamtime_id} $fsize $nfiles  $nthreads $mode  $timeout