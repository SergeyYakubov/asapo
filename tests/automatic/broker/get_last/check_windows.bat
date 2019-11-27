SET database_name=data_stream
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.data_default.insert({"_id":1}) | %mongo_exe% %database_name%  || goto :error
echo db.data_default.insert({"_id":2}) | %mongo_exe% %database_name%  || goto :error

set full_name="%1"
set short_name="%~nx1"

"%2" token -secret auth_secret.key data > token
set /P token=< token

start /B "" "%full_name%" -config settings.json
ping 1.0.0.0 -n 1 -w 100 > nul

C:\Curl\curl.exe -d '' --silent 127.0.0.1:5005/creategroup > groupid
set /P groupid=< groupid


C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/%groupid%/last?token=%token% --stderr - | findstr /c:\"_id\":2  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/%groupid%/last?token=%token% --stderr - | findstr /c:\"_id\":2  || goto :error

echo db.data_default.insert({"_id":3}) | %mongo_exe% %database_name%  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/%groupid%/last?token=%token% --stderr - | findstr /c:\"_id\":3  || goto :error

echo db.data_default.insert({"_id":4}) | %mongo_exe% %database_name%  || goto :error

C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":4  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/%groupid%/last?token=%token% --stderr - | findstr /c:\"_id\":4  || goto :error


C:\Curl\curl.exe -d '' --silent 127.0.0.1:5005/creategroup > groupid
set /P groupid=< groupid
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":1  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/stream/%groupid%/last?token=%token% --stderr - | findstr /c:\"_id\":4  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
echo db.dropDatabase() | %mongo_exe% %database_name%
del /f token
del /f groupid