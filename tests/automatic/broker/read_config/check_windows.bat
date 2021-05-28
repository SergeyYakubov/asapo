set full_name="%1"

%full_name% -config settings_bad.json 2>&1 | findstr /c:"not set"  || goto :error

%full_name% -config settings_notexist.json 2>&1| findstr cannot  || goto :error

exit /b 0

:error
exit /b 1
