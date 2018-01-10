#!/usr/bin/env bash

database_name=data

echo "db.test.deleteMany({})" | mongo ${database_name}
