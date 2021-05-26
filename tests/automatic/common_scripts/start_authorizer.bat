c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run nginx.nmd

ping 192.0.2.1 -n 1 -w 3000 > nul

set i=0
:repeat
set /a i=%i%+1
echo %i%
if %i% EQU 20 (
    goto :error
)
ping 192.0.2.1 -n 1 -w 1000 >nul
curl --silent --data "" 127.0.0.1:8400/asapo-authorizer/authorize --stderr -  | findstr /c:"Bad Request" || goto :repeat
echo asapo services ready

goto :clean

:error
echo error starting  asapo services
call :clean
exit /b 1

:clean
