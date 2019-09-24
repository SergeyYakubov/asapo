#!/usr/bin/env bash

nomad stop asapo-filemon
nomad stop asapo-filemon_batch
nomad stop asapo-filemon_multisource
nomad run asapo-test_filemon_multisource.nomad
sleep 1
nomad stop asapo-test
. ./clean_after_tests.sh

nomad run asapo-test_filegen_consumer_1M_multisource.nomad

