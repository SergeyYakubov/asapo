SET source_path=dummy

SET beamtime_id=test_run
SET data_source=detector
SET database_name=%beamtime_id%_%data_source%

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
set token_test_run=K38Mqc90iRv8fC7prcFHd994mF_wfUiJnWBfIjIzieo=

call start_services.bat

for /l %%x in (1, 1, 3) do echo db.data_default.insert({"_id":%%x,"size":100,"name":"%%x","timestamp":0,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}) | %mongo_exe% %database_name%  || goto :error


"%1" 127.0.0.1:8400 %source_path% %beamtime_id% 1 %token_test_run% 12000 1 | findstr /c:"Processed 3 file" || goto :error
goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
echo db.dropDatabase() | %mongo_exe% %database_name%
