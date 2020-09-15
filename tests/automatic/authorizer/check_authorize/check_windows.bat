set full_name="%1"
set short_name="%~nx1"

start /B "" "%full_name%" -config settings.json

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir asap3\petra3\gpfs\p01\2019\comissioning\c20180508-000-COM20181
mkdir beamline\p07\current
copy beamtime-metadata* beamline\p07\current\ /y

C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%c20180508-000-COM20181%%%%stream%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr c20180508-000-COM20181  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%c20180508-000-COM20181%%auto%%stream%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr p01  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%c20180508-000-COM20181%%%%stream%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr stream  || goto :error

C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%c20180508-000-COM20181%%%%stream%%onm80KQF8s6d2p_laW0S5IYanUUsLcnB3QO-6QQ1M90=\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr stream  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%c20180508-000-COM20181%%%%stream%%wrong\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr 401  || goto :error

C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%auto%%p07%%stream%%-pZmisCNjAbjT2gFBKs3OB2kNOU79SNsfHud0bV8gS4=\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr 11111111  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
Taskkill /IM "%short_name%" /F
rmdir /S /Q asap3
rmdir /S /Q beamline
