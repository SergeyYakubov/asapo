SET database_name=data_stream
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.meta.insert({"_id":0}) | %mongo_exe% %database_name%  || goto :error

set full_name="%1"
set short_name="%~nx1"

"%2" token -secret auth_secret.key data > token
set /P token=< token

start /B "" "%full_name%" -config settings.json

ping 1.0.0.0 -n 1 -w 100 > nul

C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/default/0/meta/0?token=%token% --stderr - | findstr /c:\"_id\":0  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/default/0/meta/1?token=%token% --stderr - | findstr /c:"no documents"  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
echo db.dropDatabase() | %mongo_exe% %database_name%
del /f token
del /f groupid