#!/usr/bin/env bash

database_name=test_run

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.dropDatabase()" | mongo ${database_name}
	rm -rf test
}

mkdir -p test
touch test/file2
sleep 0.1
touch test/file1

$@ test test_run 127.0.0.1

echo "show collections" | mongo ${database_name} | grep data
echo "db.data_default.find({"_id":1})" | mongo ${database_name} | grep file2
echo "db.data_default.find({"_id":2})" | mongo ${database_name} | grep file1

# check if gives error on duplicates
! $@ test test_run 127.0.0.1

# check if does not give error on duplicates when a flag is set
$@ -i test test_run 127.0.0.1
