SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"
SET beamtime_id=asapo_test
SET beamline=test
SET data_source=c
SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"
SET dbname=%beamtime_id%_%data_source%

mkdir %receiver_folder%

echo test > file1

ping 192.0.2.1 -n 1 -w 1000 > nul

"%1" "127.0.0.1:8400" %data_source% %beamtime_id%   > out
type out

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %receiver_root_folder%
echo db.dropDatabase() | %mongo_exe% %dbname%


