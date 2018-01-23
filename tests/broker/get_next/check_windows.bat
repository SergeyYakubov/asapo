SET database_name=data
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"

echo db.data.insert({"_id":1}) | %mongo_exe% %database_name% | findstr file2  || goto :error
echo db.data.insert({"_id":2}) | %mongo_exe% %database_name% | findstr file1  || goto :error

start /B /CMD %1

ping 1.0.0.0 -n 1 -w 100 > nul

C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/next?database=data --stderr - | findstr '"_id":1'  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/next?database=data --stderr - | findstr '"_id":2'  || goto :error
C:\Curl\curl.exe -v  --silent 127.0.0.1:5005/next?database=data --stderr - | findstr  "No Content"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM %1 /F
echo db.data.deleteMany({}) | %mongo_exe% %database_name%
echo db.current_location.deleteMany({}) | %mongo_exe% %database_name%
