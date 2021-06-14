SET database_name=data_detector
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.meta.insert({"_id":"bt"}) | %mongo_exe% %database_name%  || goto :error
echo db.meta.insert({"_id":"st_test"}) | %mongo_exe% %database_name%  || goto :error

curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.3 > broker
set /P broker=< broker

set token=%BT_DATA_TOKEN%


C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/0/meta/0?token=%token% --stderr - | findstr /c:\"_id\":\"bt\"  || goto :error
C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/test/0/meta/1?token=%token% --stderr - | findstr /c:\"_id\":\"st_test\"  || goto :error
C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/0/meta/1?token=%token% --stderr - | findstr /c:"no documents"  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
echo db.dropDatabase() | %mongo_exe% %database_name%
del /f groupid
