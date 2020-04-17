SET source_path=.
SET beamtime_id=asapo_test
SET stream_in=detector

SET indatabase_name=%beamtime_id%_%stream_in%

SET token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=

SET beamline=test

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

call start_services.bat

"%1" 127.0.0.1:8400 "%5" %beamtime_id% %token%

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
echo db.dropDatabase() | %mongo_exe% %indatabase_name%
