#!/usr/bin/env bash

mkdir -p test/subtest/subtest2
touch test/2
sleep 0.1
touch test/3
sleep 0.1
touch test/subtest/subtest2/4
sleep 0.1
touch test/1

mkdir test_noaccess1
chmod -rx test_noaccess1

