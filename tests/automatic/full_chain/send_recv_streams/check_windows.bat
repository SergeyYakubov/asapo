SET source_path=.
SET beamtime_id=asapo_test
SET stream_in=detector

SET indatabase_name=%beamtime_id%_%stream_in%

SET token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMTRhcDQzaXB0M2E0bmNpMDkwMCIsInN1YiI6ImJ0X2FzYXBvX3Rlc3QiLCJFeHRyYUNsYWltcyI6eyJBY2Nlc3NUeXBlIjoicmVhZCJ9fQ.X5Up3PBd81i4X7wUBXGkIrLEVSL-WO9kijDtzOqasgg

SET beamline=test

SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

call start_services.bat

"%1" 127.0.0.1:8400 %beamtime_id% %token%

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat

echo db.dropDatabase() | %mongo_exe% %indatabase_name%
