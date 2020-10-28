set full_name="%1"
set short_name="%~nx1"

start /B "" "%full_name%" -config settings_good.json

ping 192.0.2.1 -n 1 -w 1000 > nul
Taskkill /IM "%short_name%" /F

%full_name% -config settings_bad.json 2>&1 | findstr /c:"not set"  || goto :error

%full_name% -config settings_notexist.json 2>&1| findstr cannot  || goto :error

exit /b 0

:error
exit /b 1
