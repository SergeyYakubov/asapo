#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	kill -9 $authorizeid
}

$@ -config settings.json  &

sleep 0.3
authorizeid=`echo $!`

curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%token","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep c20180508-000-COM20181
curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%token","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep p01
curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%token","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep stream

