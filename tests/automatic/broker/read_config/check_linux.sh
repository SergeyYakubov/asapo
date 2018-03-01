#!/usr/bin/env bash

set -e
database_name=data

$@ settings_good.json &
sleep 0.3
brokerid=`echo $!`
kill -9 $brokerid

# check if gives error with bad json file
$@ settings_bad.json 2>&1 | grep "not set"

# check if gives error non-existing file
$@ settings_notexist.json 2>&1 | grep "no such"
