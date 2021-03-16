SET database_name=data_source
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.data_default.insert({"_id":1}) | %mongo_exe% %database_name%  || goto :error
echo db.data_default.insert({"_id":2}) | %mongo_exe% %database_name%  || goto :error

set full_name="%1"
set short_name="%~nx1"

set token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2MzA5NDIxMjcsImp0aSI6ImMxNGViYnJpcHQzZHQ4Y2JhczUwIiwic3ViIjoiYnRfZGF0YSIsIkV4dHJhQ2xhaW1zIjp7IkFjY2Vzc1R5cGUiOiJyZWFkIn19.U776By_privbW9WbQCSTmk9hLZVTXzTWNNap1XOIFlM

c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run nginx.nmd
ping 192.0.2.1 -n 1 -w 3000 > nul


start /B "" "%full_name%" -config settings.json

ping 192.0.2.1 -n 1 -w 1000 > nul

C:\Curl\curl.exe -d '' --silent 127.0.0.1:5005/creategroup > groupid
set /P groupid=< groupid
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/source/default/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":1  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/source/default/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":2  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/source/default/%groupid%/next?token=%token% --stderr - | findstr  /c:\"id_max\":2  || goto :error

C:\Curl\curl.exe -d '' --silent 127.0.0.1:5005/creategroup > groupid
set /P groupid=< groupid
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/database/data/source/default/%groupid%/next?token=%token% --stderr - | findstr /c:\"_id\":1  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
echo db.dropDatabase() | %mongo_exe% %database_name%
del /f token
del /f groupid
c:\opt\consul\nomad stop authorizer
c:\opt\consul\nomad stop nginx
c:\opt\consul\nomad run nginx_kill.nmd  && c:\opt\consul\nomad stop -yes -purge nginx_kill
