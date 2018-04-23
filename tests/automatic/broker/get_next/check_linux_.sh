#!/usr/bin/env bash

database_name=data

echo "db.data.insert({"_id":2})" | mongo ${database_name}
echo "db.data.insert({"_id":1})" | mongo ${database_name}

