set full_name="%1"
set short_name="%~nx1"

mkdir C:\tmp\asapo\asap3\petra3\gpfs\p00\2019\comissioning\c20180508-000-COM20181
mkdir C:\tmp\asapo\beamline\p07\current
copy beamtime-metadata-11111111.json C:\tmp\asapo\beamline\p07\current\ /y

C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"processed%%instance%%step%%c20180508-000-COM20181%%%%detector%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr c20180508-000-COM20181  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"processed%%instance%%step%%c20180508-000-COM20181%%auto%%detector%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr p00  || goto :error
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"processed%%instance%%step%%c20180508-000-COM20181%%%%detector%%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr detector  || goto :error

C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%%instance%%step%%c20180508-000-COM20181%%%%detector%%wrong\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr 401  || goto :error

set token=%BLP07_TOKEN%
C:\Curl\curl.exe -v  --silent --data "{\"SourceCredentials\":\"raw%%instance%%step%%auto%%p07%%detector%%%token%\",\"OriginHost\":\"127.0.0.1:5555\"}" 127.0.0.1:5007/authorize --stderr - | findstr 11111111  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q C:\tmp\asapo\asap3
rmdir /S /Q C:\tmp\asapo\beamline
