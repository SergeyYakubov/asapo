#!/usr/bin/env bash

mkdir -p test/subtest1/subtest2
sleep 0.1
mkdir -p test/subtest3/subtest4/
sleep 0.1

mkdir test_noaccess1
chmod -rx test_noaccess1

