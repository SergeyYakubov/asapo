#!/usr/bin/env bash

set -e

mkdir -p test
touch test/1

./worker_processfolder test | grep "Processed 1 file(s)"

rm -rf test
