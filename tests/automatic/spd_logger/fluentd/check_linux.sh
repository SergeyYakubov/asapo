#!/usr/bin/env bash

set -e

rm -f /tmp/fluentd/asapo.*.log

$@

sleep 5

cat /tmp/fluentd/asapo.*.log

res=`cat /tmp/fluentd/asapo.*.log`

echo $res | grep '"json_test":"info"'
echo $res | grep "test error"
echo $res | grep "test debug"
echo $res | grep "test warning"

