#!/usr/bin/env bash

set -e

trap Cleanup EXIT

Cleanup() {
	echo cleanup
	kill -9 $authorizeid
	sleep 1
}

$@ -config settings.json &
sleep 1
authorizeid=`echo $!`

mkdir -p asap3/petra3/gpfs/p01/2019/comissioning/c20180508-000-COM20181
mkdir -p asap3/petra3/gpfs/p01/2019/data/11000015
curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep c20180508-000-COM20181
curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep p01
curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%","OriginHost":"127.0.0.1:5555"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep stream

token=onm80KQF8s6d2p_laW0S5IYanUUsLcnB3QO-6QQ1M90= #token for c20180508-000-COM20181

curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%onm80KQF8s6d2p_laW0S5IYanUUsLcnB3QO-6QQ1M90=","OriginHost":"bla"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep stream
curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%onm80KQF8s6d2p_laW0S5IYanUUsLcnB3QO-6QQ1M90=","OriginHost":"bla"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep p01
curl -v --silent --data '{"SourceCredentials":"c20180508-000-COM20181%stream%bla","OriginHost":"bla"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep 401

token=dccMd3NT89i32Whz7yD4VQhmEJy6Kxc35wsBbWJLXp0= #token for 11000015
curl -v --silent --data '{"SourceCredentials":"11000015%stream%dccMd3NT89i32Whz7yD4VQhmEJy6Kxc35wsBbWJLXp0=","OriginHost":"bla"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep p01

token=Jaas_xTpkB0Zy5dFwjs4kCrY7yXMfbnW8Ca1aYhyKBs= #token for 11000016
curl -v --silent --data '{"SourceCredentials":"11000016%stream%Jaas_xTpkB0Zy5dFwjs4kCrY7yXMfbnW8Ca1aYhyKBs=","OriginHost":"bla"}' 127.0.0.1:5007/authorize --stderr -  | tee /dev/stderr  | grep 401

rm -rf asap3