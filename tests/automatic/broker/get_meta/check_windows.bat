SET database_name=data_detector
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.meta.insert({"_id":0}) | %mongo_exe% %database_name%  || goto :error

set full_name="%1"
set short_name="%~nx1"

c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run nginx.nmd
ping 192.0.2.1 -n 1 -w 3000 > nul


set token=%BT_DATA_TOKEN%

start /B "" "%full_name%" -config settings.json

ping 192.0.2.1 -n 1 -w 1000 > nul

C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/v0.1/beamtime/data/detector/default/0/meta/0?token=%token% --stderr - | findstr /c:\"_id\":0  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/v0.1/beamtime/data/detector/default/0/meta/1?token=%token% --stderr - | findstr /c:"no documents"  || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
echo db.dropDatabase() | %mongo_exe% %database_name%
del /f groupid
c:\opt\consul\nomad stop authorizer
c:\opt\consul\nomad stop nginx
c:\opt\consul\nomad run nginx_kill.nmd  && c:\opt\consul\nomad stop -yes -purge nginx_kill
