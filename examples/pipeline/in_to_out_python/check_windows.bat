SET source_path=.
SET beamtime_id=asapo_test
SET stream_in=detector
SET stream_out=stream

SET indatabase_name=%beamtime_id%_%stream_in%
SET outdatabase_name=%beamtime_id%_%stream_out%

SET token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=

SET beamline=test

SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\%beamline%\%beamtime_id%"


SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"

SET timeout=2
SET timeout_producer=25
SET nthreads=4

c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd
c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd

ping 1.0.0.0 -n 10 -w 100 > nul

for /l %%x in (1, 1, 3) do echo db.data.insert({"_id":%%x,"size":6,"name":"file%%x","lastchange":1,"source":"none","buf_id":0,"meta":{"test":10}}) | %mongo_exe% %indatabase_name%  || goto :error

mkdir %receiver_folder%

echo hello1 > file1
echo hello2 > file2
echo hello3 > file3

set PYTHONPATH=%2;%3

"%1" "%4" 127.0.0.1:8400 %source_path% %beamtime_id% %stream_in% %stream_out% %token% %timeout% %timeout_producer% %nthreads% 1  > out

type out
findstr /I /L /C:"Processed 3 file(s)" out || goto :error
findstr /I /L /C:"Sent 3 file(s)" out || goto :error

echo db.data.find({"_id":1}) | %mongo_exe% %outdatabase_name% | findstr  /c:"file1_%stream_out%"  || goto :error

findstr /I /L /C:"hello1" %receiver_folder%\file1_%stream_out% || goto :error
findstr /I /L /C:"hello2" %receiver_folder%\file2_%stream_out% || goto :error
findstr /I /L /C:"hello3" %receiver_folder%\file3_%stream_out% || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
c:\opt\consul\nomad stop discovery
c:\opt\consul\nomad stop broker
c:\opt\consul\nomad stop nginx
c:\opt\consul\nomad run nginx_kill.nmd  && c:\opt\consul\nomad stop -yes -purge nginx_kill
c:\opt\consul\nomad stop receiver
c:\opt\consul\nomad stop authorizer

echo db.dropDatabase() | %mongo_exe% %indatabase_name%
echo db.dropDatabase() | %mongo_exe% %outdatabase_name%
rmdir /S /Q %receiver_root_folder%
del file1 file2 file3 out
