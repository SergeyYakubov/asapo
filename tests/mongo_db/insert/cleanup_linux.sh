#!/usr/bin/env bash

database_name=data

echo "db.dropDatabase()" | mongo ${database_name}
