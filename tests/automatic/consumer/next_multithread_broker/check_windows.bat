SET database_name=test_run_detector
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
set token_test_run=%BT_TEST_RUN_TOKEN%

for /l %%x in (1, 1, 10) do echo db.data_default.insert({"_id":%%x,"size":100,"name":"%%x","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error


%1 127.0.0.1:8400 test_run 4 10 %token_test_run% || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean

echo db.dropDatabase() | %mongo_exe% %database_name%
