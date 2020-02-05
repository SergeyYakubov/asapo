SET source_path=.
SET beamtime_id=asapo_test
SET stream_in=detector

SET indatabase_name=%beamtime_id%_%stream_in%

SET token=IEfwsWa0GXky2S3MkxJSUHJT1sI8DD5teRdjBUXVRxk=

SET beamline=test

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"


c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd
c:\opt\consul\nomad run receiver.nmd
c:\opt\consul\nomad run authorizer.nmd

"%1" 127.0.0.1:8400 %beamtime_id% %token%

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
