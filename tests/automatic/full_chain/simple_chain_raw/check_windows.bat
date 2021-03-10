SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=11111111

mkdir asap3\petra3\gpfs\p07\2019\data\11111111
mkdir beamline\p07\current
copy beamtime-metadata* beamline\p07\current\ /y
copy beamtime-metadata* asap3\petra3\gpfs\p07\2019\data\11111111\ /y

set proxy_address="127.0.0.1:8400"

call start_services.bat

"%3" token -endpoint http://127.0.0.1:8400/asapo-authorizer -secret admin_token.key -type read %beamtime_id% > token

set /P token=< token


REM producer
mkdir %receiver_folder%
start /B "" "%1" %proxy_address% %beamtime_id% 100 10 4 100 100
ping 192.0.2.1 -n 1 -w 1000 > nul

REM consumer
"%2" %proxy_address% "_" %beamtime_id% 2 %token% 5000  1 > out.txt
type out.txt
findstr /i /l /c:"Processed 10 file(s)"  out.txt || goto :error
if not exist beamline\p07\current\raw\1  goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
call stop_services.bat
rmdir /S /Q asap3
rmdir /S /Q beamline
del /f token
del /f out.txt
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


