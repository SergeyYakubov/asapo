SET source_path=.
SET beamtime_id=asapo_test
SET data_source_in=detector
SET data_source_out=simulation

SET indatabase_name=%beamtime_id%_%data_source_in%
SET outdatabase_name=%beamtime_id%_%data_source_out%

SET token=%ASAPO_TEST_RW_TOKEN%

SET beamline=test

SET receiver_root_folder=c:\tmp\asapo\receiver\files
SET receiver_folder="%receiver_root_folder%\test_facility\gpfs\%beamline%\2019\data\%beamtime_id%"


SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

SET timeout=2
SET timeout_producer=25
SET nthreads=4

for /l %%x in (1, 1, 3) do echo db.data_default.insert({"_id":%%x,"size":6,"name":"processed\\file%%x","timestamp":1,"source":"none","buf_id":0,"dataset_substream":0,"meta":{"test":10}}) | %mongo_exe% %indatabase_name%  || goto :error

mkdir %receiver_folder%
mkdir processed

echo hello1 > processed\file1
echo hello2 > processed\file2
echo hello3 > processed\file3

set PYTHONPATH=%2;%3

"%1" "%4" 127.0.0.1:8400 %source_path% %beamtime_id% %data_source_in% %data_source_out% %token% %timeout% %timeout_producer% %nthreads% 1  > out

type out
findstr /I /L /C:"Processed 3 file(s)" out || goto :error
findstr /I /L /C:"Sent 3 file(s)" out || goto :error

echo db.data_default.find({"_id":1}) | %mongo_exe% %outdatabase_name% | findstr  /c:"file1_%data_source_out%"  || goto :error

findstr /I /L /C:"hello1" %receiver_folder%\processed\file1_%data_source_out% || goto :error
findstr /I /L /C:"hello2" %receiver_folder%\processed\file2_%data_source_out% || goto :error
findstr /I /L /C:"hello3" %receiver_folder%\processed\file3_%data_source_out% || goto :error


goto :clean

:error
call :clean
exit /b 1

:clean
echo db.dropDatabase() | %mongo_exe% %indatabase_name%
echo db.dropDatabase() | %mongo_exe% %outdatabase_name%
rmdir /S /Q %receiver_root_folder%
rmdir /S /Q processed
