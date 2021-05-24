c:\opt\consul\nomad run receiver_tcp.nmd
c:\opt\consul\nomad run authorizer.nmd
c:\opt\consul\nomad run discovery.nmd
c:\opt\consul\nomad run broker.nmd
c:\opt\consul\nomad run nginx.nmd
c:\opt\consul\nomad run file_transfer.nmd


ping 192.0.2.1 -n 1 -w 3000 > nul

set i=0
:repeat
set /a i=%i%+1
echo %i%
if %i% EQU 20 (
    goto :error
)
ping 192.0.2.1 -n 1 -w 1000 >nul
curl --silent --fail 127.0.0.1:8400/asapo-discovery/v0.1/asapo-receiver?protocol=v0.1 --stderr - | findstr 127.0.0.1  || goto :repeat
curl --silent --fail 127.0.0.1:8400/asapo-discovery/v0.1/asapo-broker?protocol=v0.1 --stderr - | findstr 127.0.0.1 || goto :repeat
curl --silent --fail 127.0.0.1:8400/asapo-discovery/v0.1/asapo-file-transfer?protocol=v0.1 --stderr -  | findstr 127.0.0.1 || goto :repeat
echo discovery ready

goto :clean

:error
echo error starting  asapo services
exit /b 1

:clean
