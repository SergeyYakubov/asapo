SET database_name=test_run
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"

::first argument  path to the executable
:: second argument path to the broker

set full_name="%2"
set short_name="%~nx2"

start /B "" "%full_name%" -config settings.json

ping 1.0.0.0 -n 1 -w 100 > nul

for /l %%x in (1, 1, 10) do echo db.data.insert({"_id":%%x,"size":100,"name":"%%x","lastchange":1}) | %mongo_exe% %database_name%  || goto :error


%1 127.0.0.1:5005 %database_name% 4 10 || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
echo db.dropDatabase() | %mongo_exe% %database_name%