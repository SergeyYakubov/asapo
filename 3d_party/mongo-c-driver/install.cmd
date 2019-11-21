:: download and untar mongoc driver sources to dir
:: https://github.com/mongodb/mongo-c-driver/releases/download/1.15.2/mongo-c-driver-1.15.2.tar.gz

:: set directory with mongoc driver sources
SET dir=c:\tmp\mongo-c-driver-1.15.2

set mypath=%cd%
cd /d %dir%

:: install libbson
cd src\libbson
cmake "-DCMAKE_INSTALL_PREFIX=C:\mongo-c-driver" ^
      "-DCMAKE_BUILD_TYPE=Release" ^
      "-DCMAKE_C_FLAGS_RELEASE=/MT"
cmake --build . --config Release
cmake --build . --target install --config Release

:: install mongoc
cd %dir%
cmake "-DCMAKE_INSTALL_PREFIX=C:\mongo-c-driver" ^
      "-DCMAKE_PREFIX_PATH=C:\mongo-c-driver" ^
      "-DCMAKE_BUILD_TYPE=Release" ^
      "-DENABLE_SSL=OFF" ^
      "-DENABLE_SASL=OFF" ^
      "-DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF" ^
      "-DMONGOC_ENABLE_STATIC=ON" ^
      "-DCMAKE_C_FLAGS_RELEASE=/MT"
cmake --build . --config Release
cmake --build . --target install --config Release

cd /d %mypath%
