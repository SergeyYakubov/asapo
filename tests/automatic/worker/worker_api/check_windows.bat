SET database_name=test_run
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"
set token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

::first argument  path to the executable

c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

for /l %%x in (1, 1, 10) do echo db.data.insert({"_id":%%x,"size":100,"name":"%%x","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

%1 127.0.0.1:8400 %database_name% %token_test_run%  single || goto :error

echo db.dropDatabase() | %mongo_exe% %database_name%

for /l %%x in (1, 1, 10) do echo db.data.insert({"_id":%%x,"size":3,"images":[{"_id":1, "size":100,"name":"%%x_1","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}},{"_id":2, "size":100,"name":"%%x_2","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}},{"_id":3, "size":100,"name":"%%x_3","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}]}) | %mongo_exe% %database_name%  || goto :error

%1 127.0.0.1:8400 %database_name% %token_test_run%  dataset || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop broker
c:\opt\consul\nomad stop nginx
echo db.dropDatabase() | %mongo_exe% %database_name%

