#!/usr/bin/env bash

set -e


root_folder=/tmp/asapo


trap Cleanup EXIT

Cleanup() {
    set +e
	echo cleanup
	rm -rf /tmp/asapo/test_in /tmp/asapo/test_out #output
	kill -9 $producer_id &>/dev/null
}

mkdir -p /tmp/asapo/test_in/test1 /tmp/asapo/test_in/test2 /tmp/asapo/test_out

$1 test.json &> output &
producer_id=`echo $!`
sleep 0.5

echo test1 > /tmp/asapo/test_in/test1/test1.dat
echo test2 > /tmp/asapo/test_in/test2/test2.tmp
mkdir -p /tmp/asapo/test_in/test2/subdir
sleep 0.1

echo test3 > /tmp/asapo/test_in/test2/subdir/test3.dat

sleep 0.1

cat /tmp/asapo/test_out/test1/test1.dat | grep test1
cat /tmp/asapo/test_out/test2/subdir/test3.dat | grep test3

test  ! -e /tmp/asapo/test_out/test2/test2.tmp

test  ! -e /tmp/asapo/test_in/test1/test1.dat
test  ! -e /tmp/asapo/test_in/test2/subdir/test3.dat


kill -s INT $producer_id
sleep 0.5
cat output
cat output | grep "Processed 2"

