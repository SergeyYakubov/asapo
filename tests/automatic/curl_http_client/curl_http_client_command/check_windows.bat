setlocal
SET beamtime_id=aaa
SET file_transfer_folder=c:\tmp\asapo\asap3\petra3\gpfs\p01\2019\data\%beamtime_id%
set file_transfer_folder=%file_transfer_folder:\=\\%

mkdir %file_transfer_folder%
echo | set /p dummyName="hello" > %file_transfer_folder%\aaa

python3 -c "import os;fout=open('%file_transfer_folder%\\random', 'wb');fout.write(os.urandom(100000))"

"%1"  127.0.0.1:5007 127.0.0.1:5008 %file_transfer_folder%   || goto :error

type bbb | findstr /c:"hello"  || goto :error

goto :clean

:error
call :clean
exit /b 1

:clean
rmdir /S /Q %file_transfer_folder%
del /f bbb random

