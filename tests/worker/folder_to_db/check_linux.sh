#!/usr/bin/env bash

set -e

mkdir -p test
touch test/1

$1 test mongodb://127.0.0.1

rm -rf test
