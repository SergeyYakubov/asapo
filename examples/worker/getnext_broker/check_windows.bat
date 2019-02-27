SET database_name=test_run
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
set token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

for /l %%x in (1, 1, 3) do echo db.data.insert({"_id":%%x,"size":100,"name":"%%x","lastchange":1,"source":"none","buf_id":0}) | %mongo_exe% %database_name%  || goto :error


"%1" 127.0.0.1:8400 %database_name% 1 %token_test_run% 1000 1 | findstr /c:"Processed 3 file" || goto :error
goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop broker
c:\opt\consul\nomad stop nginx
echo db.dropDatabase() | %mongo_exe% %database_name%
