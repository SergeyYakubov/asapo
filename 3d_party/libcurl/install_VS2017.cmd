:: download and untar libcurl sources to dir
:: https://curl.haxx.se/download/curl-7.58.0.tar.gz

:: set directory with libcurl sources
SET dir=c:\tmp\curl-7.58.0
:: set directory where libcurl should be installed
SET install_dir=c:\Curl

set mypath=%cd%

call "c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

cd /d %dir%\winbuild

nmake.exe /f Makefile.vc mode=static VC=14 MACHINE=X86 RTLIBCFG=static

xcopy /isvy %dir%\builds\libcurl-vc14-X86-release-static-ipv6-sspi-winssl\include %install_dir%\include
xcopy /isvy %dir%\builds\libcurl-vc14-X86-release-static-ipv6-sspi-winssl\lib %install_dir%\lib
rename %install_dir%\lib\libcurl_a.lib libcurl.lib


