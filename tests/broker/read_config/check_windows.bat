set full_name="%1"
set short_name="%~nx1"

start /B "" "%full_name%" settings_good.json

ping 1.0.0.0 -n 1 -w 100 > nul
Taskkill /IM "%short_name%" /F

%full_name% settings_bad.json 2>&1 | findstr invalid  || goto :error

%full_name% settings_notexist.json 2>&1| findstr cannot  || goto :error

exit /b 0

:error
exit /b 1
