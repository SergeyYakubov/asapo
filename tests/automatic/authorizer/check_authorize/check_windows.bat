set full_name="%1"
set short_name="%~nx1"

start /B "" "%full_name%" -config settings.json

ping 1.0.0.0 -n 1 -w 100 > nul

C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"c20180508-000-COM20181%%stream%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr c20180508-000-COM20181  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"c20180508-000-COM20181%%stream%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr p01  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"c20180508-000-COM20181%%stream%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr stream  || goto :error


C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"c20180508-000-COM20181%%stream%%onm80KQF8s6d2p_laW0S5IYanUUsLcnB3QO-6QQ1M90=\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr stream  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"c20180508-000-COM20181%%stream%%wrong\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr 401  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
