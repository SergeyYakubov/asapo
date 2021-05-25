SET database_name=data_detector
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.data_default.insert({"_id":1}) | %mongo_exe% %database_name%  || goto :error
echo db.data_default.insert({"_id":2}) | %mongo_exe% %database_name%  || goto :error

set token=%BT_DATA_TOKEN%

curl --silent 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.3 > broker
set /P broker=< broker

C:\Curl\curl.exe -d '' --silent %broker%/v0.2/creategroup > groupid
set /P groupid=< groupid


C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/0/last?token=%token% --stderr - | findstr /c:\"_id\":2  || goto :error
C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/0/last?token=%token% --stderr - | findstr /c:\"_id\":2  || goto :error

echo db.data_default.insert({"_id":3}) | %mongo_exe% %database_name%  || goto :error
C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/0/last?token=%token% --stderr - | findstr /c:\"_id\":3  || goto :error

echo db.data_default.insert({"_id":4}) | %mongo_exe% %database_name%  || goto :error

C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":1  || goto :error
C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/0/last?token=%token% --stderr - | findstr /c:\"_id\":4  || goto :error


C:\Curl\curl.exe -d '' --silent %broker%/v0.2/creategroup > groupid
set /P groupid=< groupid
C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":1  || goto :error
C:\Curl\curl.exe -v  --silent %broker%/v0.2/beamtime/data/detector/default/0/last?token=%token% --stderr - | findstr /c:\"_id\":4  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
echo db.dropDatabase() | %mongo_exe% %database_name%
del /f groupid
del /f broker
