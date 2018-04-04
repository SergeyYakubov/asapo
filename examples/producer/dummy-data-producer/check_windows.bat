"%1" 0.0.0.0 1 1 2>&1 | findstr "refused" || goto :error
goto :clean

:error
call :clean
exit /b 1

:clean

