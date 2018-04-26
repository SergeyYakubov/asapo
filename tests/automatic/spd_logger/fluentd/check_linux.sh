#!/usr/bin/env bash

set -e

rm -f /tmp/fluentd/asapo.*.log

$@

sleep 5

cat /tmp/fluentd/asapo.*.log

res=`cat /tmp/fluentd/asapo.*.log`

echo $res | grep "test_info"
echo $res | grep "test_error"
echo $res | grep "test_debug"
echo $res | grep "test_warning"

