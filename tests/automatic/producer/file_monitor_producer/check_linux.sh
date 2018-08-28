#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	rm -rf test_in test_out #output
	kill -9 $producer_id &>/dev/null
}

mkdir -p test_in test_out

$1 test.json &> output &
producer_id=`echo $!`
sleep 0.1

echo test1 > test_in/test1.dat
echo test2 > test_in/test2.tmp
#cp test_in/test2.tmp test_in/test4.dat
mkdir test_in/subdir
echo test3 > test_in/subdir/test3.dat

sleep 0.1

cat test_out/test1.dat | grep test1
cat test_out/subdir/test3.dat | grep test3

test  ! -e test_out/test2.tmp

kill -s INT $producer_id
sleep 0.5
cat output
cat output | grep processed 2

