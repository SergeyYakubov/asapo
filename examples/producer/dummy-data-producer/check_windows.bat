"%1" 0.0.0.0 1 1 1 1 2>&1 | findstr "not successfully" || goto :error
goto :clean

:error
call :clean
exit /b 1

:clean

