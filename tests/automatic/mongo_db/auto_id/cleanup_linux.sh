#!/usr/bin/env bash

database_name=data_%2F%20%5C%2E%22%24

echo "db.dropDatabase()" | mongo ${database_name}
