#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
    set +e
	echo cleanup
	rm -rf /tmp/test_in /tmp/test_out output
	kill -9 $producer_id &>/dev/null
}

mkdir -p /tmp/test_in/test1 /tmp/test_in/test2 /tmp/test_out

$1 test.json &> output &
producer_id=`echo $!`
sleep 0.5

echo test1 > /tmp/test_in/test1/test1.dat
echo test2 > /tmp/test_in/test2/test2.tmp
mkdir -p /tmp/test_in/test2/subdir
echo test3 > /tmp/test_in/test2/subdir/test3.dat

sleep 0.1

cat /tmp/test_out/test1/test1.dat | grep test1
cat /tmp/test_out/test2/subdir/test3.dat | grep test3

test  ! -e /tmp/test_out/test2/test2.tmp

kill -s INT $producer_id
sleep 0.5
cat output
cat output | grep "Processed 2"

