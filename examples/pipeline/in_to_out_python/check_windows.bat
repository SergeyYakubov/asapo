SET source_path=.
SET beamtime_id=asapo_test
SET stream_in=detector
SET stream_out=stream

SET indatabase_name=%beamtime_id%_%stream_in%
SET outdatabase_name=%beamtime_id%_%stream_out%

SET token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=

SET beamline=test

SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"


SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

SET timeout=2
SET timeout_producer=25
SET nthreads=4

call start_services.bat

for /l %%x in (1, 1, 3) do echo db.data_default.insert({"_id":%%x,"size":6,"name":"processed\\file%%x","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %indatabase_name%  || goto :error

mkdir %receiver_folder%
mkdir processed

echo hello1 > processed\file1
echo hello2 > processed\file2
echo hello3 > processed\file3

set PYTHONPATH=%2;%3

"%1" "%4" 127.0.0.1:8400 %source_path% %beamtime_id% %stream_in% %stream_out% %token% %timeout% %timeout_producer% %nthreads% 1  > out

type out
findstr /I /L /C:"Processed 3 file(s)" out || goto :error
findstr /I /L /C:"Sent 3 file(s)" out || goto :error

echo db.data_default.find({"_id":1}) | %mongo_exe% %outdatabase_name% | findstr  /c:"file1_%stream_out%"  || goto :error

findstr /I /L /C:"hello1" %receiver_folder%\processed\file1_%stream_out% || goto :error
findstr /I /L /C:"hello2" %receiver_folder%\processed\file2_%stream_out% || goto :error
findstr /I /L /C:"hello3" %receiver_folder%\processed\file3_%stream_out% || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
echo db.dropDatabase() | %mongo_exe% %indatabase_name%
echo db.dropDatabase() | %mongo_exe% %outdatabase_name%
rmdir /S /Q %receiver_root_folder%
rmdir /S /Q processed
