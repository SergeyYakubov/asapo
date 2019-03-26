SET database_name=data
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"

echo db.data.insert({"_id":1}) | %mongo_exe% %database_name%  || goto :error
echo db.data.insert({"_id":2}) | %mongo_exe% %database_name%  || goto :error

set full_name="%1"
set short_name="%~nx1"

"%2" token -secret broker_secret.key data > token
set /P token=< token


curl -d '' --silent 127.0.0.1:5005/creategroup > groupid
set /P groupid=< groupid


start /B "" "%full_name%" -config settings.json

ping 1.0.0.0 -n 1 -w 100 > nul

C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":1  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":2  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/%groupid%/next?token=%token% --stderr - | findstr  /c:"not found"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
echo db.dropDatabase() | %mongo_exe% %database_name%
del /f token
del /f groupid