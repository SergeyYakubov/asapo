SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"


mkdir %receiver_folder%

"%1" 127.0.0.1:8400 %beamtime_id% 100 1 1 0 30 3

ping 192.0.2.1 -n 1 -w 1000 > nul

FOR /F "usebackq" %%A IN ('%receiver_folder%\processed\1_1') DO set size=%%~zA
if %size% NEQ 100000 goto :error

FOR /F "usebackq" %%A IN ('%receiver_folder%\processed\1_2') DO set size=%%~zA
if %size% NEQ 100000 goto :error

FOR /F "usebackq" %%A IN ('%receiver_folder%\processed\1_3') DO set size=%%~zA
if %size% NEQ 100000 goto :error


echo db.data_default.find({"messages._id":{$gt:0}},{"messages.name":1}) | %mongo_exe% %beamtime_id%_detector | findstr 1_1  || goto :error
echo db.data_default.find({"messages._id":{$gt:0}},{"messages.name":1}) | %mongo_exe% %beamtime_id%_detector | findstr 1_2  || goto :error
echo db.data_default.find({"messages._id":{$gt:0}},{"messages.name":1}) | %mongo_exe% %beamtime_id%_detector | findstr 1_3  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %beamtime_id%_detector


