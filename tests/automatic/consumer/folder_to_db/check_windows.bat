SET database_name=test_run
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

mkdir test
echo "" > test/file2
ping 192.0.2.1 -n 1 -w 1000 > nul
echo "" > test/file1

%* test test_run 127.0.0.1 || goto :error

echo show collections | %mongo_exe% %database_name% | findstr data  || goto :error
echo db.data_default.find({"_id":1}) | %mongo_exe% %database_name% | findstr file2  || goto :error
echo db.data_default.find({"_id":2}) | %mongo_exe% %database_name% | findstr file1  || goto :error

# check if gives error on duplicates
%* test test_run 127.0.0.1  && goto :error

# check if does not give error on duplicates when a flag is set
 %* -i test test_run 127.0.0.1  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
echo db.dropDatabase() | %mongo_exe% %database_name%
rmdir /S /Q test
