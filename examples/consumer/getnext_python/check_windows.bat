SET source_path=dummy
SET beamtime_id=test_run
SET data_source=detector
SET database_name=%beamtime_id%_%data_source%

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
set token_test_run=%BT_TEST_RUN_TOKEN%
set group_id=bif31l2uiddd4r0q6b40

for /l %%x in (1, 1, 3) do echo db.data_default.insert({"_id":%%x,"size":100,"name":"%%x","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error


echo db.meta.insert({"_id":0,"meta_test":"test"}) | %mongo_exe% %database_name%  || goto :error

set PYTHONPATH=%1

python3 getnext.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run% %group_id% > out
type out
type out | findstr /c:"100" || goto :error
type out | findstr /c:"\"_id\": 1" || goto :error
type out | findstr /c:"\"meta_test\": \"test\"" || goto :error

python3 getnext.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run% %group_id% > out
type out
type out | findstr /c:"\"_id\": 2" || goto :error

python3 getnext.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run% %group_id% > out
type out
type out | findstr /c:"\"_id\": 3" || goto :error


python3 getnext.py 127.0.0.1:8400 %source_path% %beamtime_id%  %token_test_run% new > out
type out
type out | findstr /c:"100" || goto :error
type out | findstr /c:"\"_id\": 1" || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
echo db.dropDatabase() | %mongo_exe% %database_name%
