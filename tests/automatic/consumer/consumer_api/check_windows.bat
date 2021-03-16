
SET beamtime_id=test_run
SET data_source=detector

SET database_name=%beamtime_id%_%data_source%
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
set token_test_run=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2MzA5NDI0MjQsImp0aSI6ImMxNGVkbTNpcHQzZHQ4Y2JhczVnIiwic3ViIjoiYnRfdGVzdF9ydW4iLCJFeHRyYUNsYWltcyI6eyJBY2Nlc3NUeXBlIjoicmVhZCJ9fQ.SBzrEy-d3ayhVZMSskYUMLM2LVHw3yiM32mIOcITh0g

call start_services.bat

for /l %%x in (1, 1, 10) do echo db.data_default.insert({"_id":%%x,"size":6,"name":"%%x","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

for /l %%x in (1, 1, 5) do echo db.data_stream1.insert({"_id":%%x,"size":6,"name":"1%%x","timestamp":1000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

echo db.data_stream1.insert({"_id":6,"size":0,"name":"asapo_finish_stream","timestamp":1000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"next_stream":"ns"}}) | %mongo_exe% %database_name%  || goto :error

for /l %%x in (1, 1, 5) do echo db.data_stream2.insert({"_id":%%x,"size":6,"name":"2%%x","timestamp":2000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

echo db.data_stream2.insert({"_id":6,"size":0,"name":"asapo_finish_stream","timestamp":2000,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"next_stream":"asapo_no_next"}}) | %mongo_exe% %database_name%  || goto :error


echo hello1 > 1


%1 127.0.0.1:8400 %beamtime_id% %token_test_run%  single || goto :error

echo db.dropDatabase() | %mongo_exe% %database_name%

for /l %%x in (1, 1, 10) do echo db.data_default.insert({"_id":%%x,"size":3,"messages":[{"_id":1, "size":6,"name":"%%x_1","timestamp":1,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}},{"_id":2, "size":6,"name":"%%x_2","timestamp":1,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}},{"_id":3, "size":6,"name":"%%x_3","timestamp":1,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}]}) | %mongo_exe% %database_name%  || goto :error

echo hello1 > 1_1

%1 127.0.0.1:8400 %beamtime_id% %token_test_run%  datasets || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
echo db.dropDatabase() | %mongo_exe% %database_name%
del "1 1_1"

