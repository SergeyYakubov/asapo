#!/usr/bin/env bash

set -e

mkdir -p test
touch test/1

$1 test 123

rm -rf test
