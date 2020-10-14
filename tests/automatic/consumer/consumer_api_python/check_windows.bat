setlocal
SET beamtime_id=test_run
SET source_path=%cd%\asap3\petra3\gpfs\p01\2019\data\%beamtime_id%
set source_path=%source_path:\=\\%

SET stream=detector

SET database_name=%beamtime_id%_%stream%

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
set token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

call start_services.bat

for /l %%x in (1, 1, 5) do echo db.data_default.insert({"_id":%%x,"size":6,"name":"%%x","timestamp":0,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

echo db.data_streamfts.insert({"_id":1,"size":0,"name":"1","timestamp":1000,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

for /l %%x in (1, 1, 5) do echo db.data_stream1.insert({"_id":%%x,"size":6,"name":"1%%x","timestamp":1000,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error

for /l %%x in (1, 1, 5) do echo db.data_stream2.insert({"_id":%%x,"size":6,"name":"2%%x","timestamp":2000,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error


mkdir %source_path%


set PYTHONPATH=%1

echo | set /p dummyName="hello1" > %source_path%\1
echo | set /p dummyName="hello1" > %source_path%\1_1

python %3/consumer_api.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run%  single || goto :error

echo db.dropDatabase() | %mongo_exe% %database_name%

for /l %%x in (1, 1, 10) do echo db.data_default.insert({"_id":%%x,"size":3,"images":[{"_id":1, "size":6,"name":"%%x_1","timestamp":0,"source":"none","buf_id":0,"meta":{"test":10}},{"_id":2, "size":6,"name":"%%x_2","timestamp":1,"source":"none","buf_id":0,"meta":{"test":10}},{"_id":3, "size":6,"name":"%%x_3","timestamp":1,"source":"none","buf_id":0,"meta":{"test":10}}]}) | %mongo_exe% %database_name%  || goto :error

python %3/consumer_api.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run% datasets || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat

echo db.dropDatabase() | %mongo_exe% %database_name%
del c:\tmp\asapo\consumer_test\files\1
del c:\tmp\asapo\consumer_test\files\1_1

