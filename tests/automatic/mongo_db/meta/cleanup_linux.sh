#!/usr/bin/env bash

database_name=test

echo "db.dropDatabase()" | mongo ${database_name}
