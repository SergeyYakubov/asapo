SET source_path=c:\\tmp\\asapo\\consumer_test\\files

SET beamtime_id=test_run
SET stream=detector

SET database_name=%beamtime_id%_%stream%

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
set token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd
c:\opt\consul\nomad run file_transfer.nmd
c:\opt\consul\nomad run authorizer.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

for /l %%x in (1, 1, 5) do echo db.data_default.insert({"_id":%%x,"size":6,"name":"%%x","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

for /l %%x in (1, 1, 5) do echo db.data_stream1.insert({"_id":%%x,"size":6,"name":"1%%x","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

for /l %%x in (1, 1, 5) do echo db.data_stream2.insert({"_id":%%x,"size":6,"name":"2%%x","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error


mkdir %source_path%


set PYTHONPATH=%1

echo | set /p dummyName="hello1" > %source_path%\1
echo | set /p dummyName="hello1" > %source_path%\1_1

python %3/consumer_api.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run%  single || goto :error

echo db.dropDatabase() | %mongo_exe% %database_name%

for /l %%x in (1, 1, 10) do echo db.data_default.insert({"_id":%%x,"size":3,"images":[{"_id":1, "size":6,"name":"%%x_1","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}},{"_id":2, "size":6,"name":"%%x_2","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}},{"_id":3, "size":6,"name":"%%x_3","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}]}) | %mongo_exe% %database_name%  || goto :error

python %3/consumer_api.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run% datasets || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop broker
c:\opt\consul\nomad stop nginx
c:\opt\consul\nomad run nginx_kill.nmd  && c:\opt\consul\nomad stop -yes -purge nginx_kill
c:\opt\consul\nomad stop file_transfer
c:\opt\consul\nomad stop authorizer

echo db.dropDatabase() | %mongo_exe% %database_name%
del c:\tmp\asapo\consumer_test\files\1
del c:\tmp\asapo\consumer_test\files\1_1

