#!/usr/bin/env bash

database_name=data

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	echo "db.test.deleteMany({})" | mongo ${database_name}
	rm -rf test
}

mkdir -p test
touch test/file2
sleep 0.1
touch test/file1

$@ test 127.0.0.1

echo "show collections" | mongo ${database_name} | grep test
echo "db.test.find({"_id":1})" | mongo ${database_name} | grep file2
echo "db.test.find({"_id":2})" | mongo ${database_name} | grep file1

# check if gives error on duplicates
! $@ test 127.0.0.1

# check if does not give error on duplicates when a flag is set
$@ -i test 127.0.0.1
